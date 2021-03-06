#include "Utils.h"
#include "UtilsCUDA.h"
#include "CornerDetector.h"

/*
	Kernel Device (GPU) variables and functions
	===========================================
*/

__global__ void applySobelKernel(byte* d_srcMatrix, byte* d_dstMatrix,
	int rows, int cols, bool isByX)
{
	const short kernelGx[CORNER_DETECTION_KERNEL_SIZE * CORNER_DETECTION_KERNEL_SIZE] =
	{
		-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1
	};

	const short kernelGy[CORNER_DETECTION_KERNEL_SIZE * CORNER_DETECTION_KERNEL_SIZE] =
	{
		-1, -2, -1,
		 0,  0,  0,
		 1,  2,  1
	};

	// Number of pixels around the central pixel for valid kernel odd sizes: 3, 5, 7
	const int halfKernelSize = (int)CORNER_DETECTION_KERNEL_SIZE / 2;

	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	if ((x >= cols) || (y >= rows))
	{
		return;
	}

	// Skip margins with zeroing of result
	if ((x < halfKernelSize) ||
		(x >= cols - halfKernelSize) ||
		(y < halfKernelSize) ||
		(y >= rows - halfKernelSize))
	{
		d_dstMatrix[y * cols + x] = 0;
		return;
	}

	short* kernel = isByX ? (short*)kernelGx : (short*)kernelGy;

	// Calculate convolution with the kernel
	short convolved = 0;
	for (int kernelY = 0; kernelY < CORNER_DETECTION_KERNEL_SIZE; kernelY++)
	{
		for (int kernelX = 0; kernelX < CORNER_DETECTION_KERNEL_SIZE; kernelX++)
		{
			int matrixY = y - halfKernelSize + kernelY;
			int matrixX = x - halfKernelSize + kernelX;
			short matrixVal = (short)d_srcMatrix[matrixY * cols + matrixX];
			short kernelVal = kernel[kernelY * CORNER_DETECTION_KERNEL_SIZE + kernelX];
			convolved += matrixVal * kernelVal;
		}
	}

	// Trim convolution result before setting to destination matrix
	if (convolved < 0)
	{
		convolved = -convolved;
	}
	if (convolved > (short)WHITE)
	{
		convolved = (short)WHITE;
	}
	d_dstMatrix[y * cols + x] = (byte)convolved;
}

__global__ void combineSobelFilters(byte* d_srcMatrixGx, byte* d_srcMatrixGy,
	byte* d_dstMatrix, int rows, int cols)
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	if ((x >= cols) || (y >= rows))
	{
		return;
	}

	short valGx = (short)d_srcMatrixGx[y * cols + x];
	short valGy = (short)d_srcMatrixGy[y * cols + x];
	short val = valGx + valGy;
	if (val > (short)WHITE)
	{
		val = (short)WHITE;
	}
	d_dstMatrix[y * cols + x] = (byte)val;
}

/*
	Public Host (CPU) functions to call kernel Device (GPU) functions
	=================================================================
*/

CornerDetector::CornerDetector()
{
	m_z = 0.0F;
	m_croppedRows = 0;
	m_gradientThreshold = 0;
	m_minDistancePixels = 0;
	m_minFoundCapillaries = 0;
}

void CornerDetector::init(Config& config)
{
	initConfig(config);
}

void CornerDetector::setLayerPosition(float z)
{
	m_z = z;
}

size_t CornerDetector::getMinFoundCapillaries()
{
	return m_minFoundCapillaries;
}

/*
	Apply Gx and Gy Sobel kernels and find average gradient values over predefined threshold
*/
std::vector<ScoredCorner> CornerDetector::getCornersSobel(Map& map, ByteMatrix& matrix,
	const std::string& capillariesFolderName, size_t layerIndex)
{
	// Number of pixels around the central pixel for valid kernel odd sizes: 3, 5, 7
	size_t halfKernelSize = CORNER_DETECTION_KERNEL_SIZE / 2;

	// Filled and returned detected corners
	std::vector<ScoredCorner> scoredCorners;

	int rows = (int)matrix.rows();
	int cols = (int)matrix.cols();

	// Parameters to launch parallel threads
	dim3 blockSize(128, 1);
	dim3 numBlocks(divideCeil(cols, blockSize.x), divideCeil(rows, blockSize.y), 1);

	// Device memory buffers
	byte* d_srcBuffer = nullptr;
	byte* d_dstBufferSobelGx = nullptr;
	byte* d_dstBufferSobelGy = nullptr;
	byte* d_dstBufferSobel = nullptr;

	// Allocate device memory buffers and fill source device buffer by processed matrix
	checkCuda(cudaMalloc(&d_srcBuffer, rows * cols));
	checkCuda(cudaMalloc(&d_dstBufferSobelGx, rows * cols));
	checkCuda(cudaMalloc(&d_dstBufferSobelGy, rows * cols));
	checkCuda(cudaMalloc(&d_dstBufferSobel, rows * cols));
	checkCuda(cudaMemcpy(d_srcBuffer, matrix.getBuffer(), rows * cols, cudaMemcpyHostToDevice));

	// Calculate connvolutions with Sobel kernels on GPU
	applySobelKernel<<<numBlocks, blockSize>>>(d_srcBuffer, d_dstBufferSobelGx,
		rows, cols, true);
	applySobelKernel<<<numBlocks, blockSize>>>(d_srcBuffer, d_dstBufferSobelGy,
		rows, cols, false);
	combineSobelFilters<<<numBlocks, blockSize>>>(d_dstBufferSobelGx, d_dstBufferSobelGy,
		d_dstBufferSobel, rows, cols);

	// Get calculated gradient from device memory
	ByteMatrix gradient = ByteMatrix(rows, cols);
	checkCuda(cudaMemcpy(gradient.getBuffer(), d_dstBufferSobel, rows * cols, cudaMemcpyDeviceToHost));
	checkCuda(cudaDeviceSynchronize());

	for (size_t row = halfKernelSize; row < rows - m_croppedRows - halfKernelSize; row++)
	{
		for (size_t col = halfKernelSize; col < cols - halfKernelSize; col++)
		{
			// Skip possible false-positive corners on seams
			if (map.isOnSeam(row, true) || map.isOnSeam(col, false))
			{
				continue;
			}

			// Skip unexpected gray levels - mainly on flares
			byte grayLevel = matrix.get(row, col);
			if (!isValidGrayLevelOriginal(grayLevel))
			{
				continue;
			}

			// Calculate the average value of gradients in the kernel
			unsigned short sumGrad = 0;
			for (short kernelRow = -1; kernelRow <= 1; kernelRow++)
			{
				for (short kernelCol = -1; kernelCol <= 1; kernelCol++)
				{
					sumGrad += (unsigned short)gradient.get(row + kernelRow, col + kernelCol);
				}
			}
			byte avgGrad = (byte)std::roundf((float)sumGrad /
				CORNER_DETECTION_KERNEL_SIZE / CORNER_DETECTION_KERNEL_SIZE);

			// If the gradient does not meet quality criteria - skip examined pixel
			if (avgGrad < m_gradientThreshold)
			{
				continue;
			}

			// Iterate previously accumulated corners to whehter near corner was added before
			bool nearCornerFound = false;
			size_t cornerIndex = 0;
			for (; cornerIndex < scoredCorners.size(); cornerIndex++)
			{
				int distPixelsX = (int)mm2pixels(scoredCorners[cornerIndex].x) - (int)col;
				int distPixelsY = (int)mm2pixels(scoredCorners[cornerIndex].y) - (int)row;
				size_t distPixels2 = (size_t)(distPixelsX * distPixelsX + distPixelsY * distPixelsY);
				if (distPixels2 < m_minDistancePixels * m_minDistancePixels)
				{
					nearCornerFound = true;
					break;
				}
			}

			// Parameters of overwritten near corner (if found) or accumulated corner
			float x = pixels2mm(col);
			float y = pixels2mm(row);
			float nomalizedScore = 100.0F * ((float)avgGrad / m_gradientThreshold - 1.0F);

			if (nearCornerFound)
			{
				// If near corner is found - overwrite by current corner if it has better score
				if (nomalizedScore > scoredCorners[cornerIndex].score)
				{
					scoredCorners[cornerIndex].x = x;
					scoredCorners[cornerIndex].y = y;
					scoredCorners[cornerIndex].score = nomalizedScore;
					scoredCorners[cornerIndex].grayLevel = grayLevel;
				}
			}
			else
			{
				// Accumulate new corner
				ScoredCorner scoredCorner(x, y, m_z, nomalizedScore, grayLevel);
				scoredCorners.push_back(scoredCorner);
			}
		}
	}

	// Sort found corners by score in descending order
	std::sort(scoredCorners.begin(), scoredCorners.end(), [](ScoredCorner cornerL, ScoredCorner cornerR) {
		return cornerL.score > cornerR.score;
	});

	checkCuda(cudaFree(d_srcBuffer));
	checkCuda(cudaFree(d_dstBufferSobelGx));
	checkCuda(cudaFree(d_dstBufferSobelGy));
	checkCuda(cudaFree(d_dstBufferSobel));

#ifdef _DEBUG
	std::string filenameGradient = capillariesFolderName + "/Gradient" + std::to_string(layerIndex + 1) + ".bmp";
	cv::imwrite(filenameGradient, gradient.asCvMatU8());
	std::string filenameLayer = capillariesFolderName + "/Layer" + std::to_string(layerIndex + 1) + ".csv";
	writeCorners(scoredCorners, filenameLayer);
#endif
	return scoredCorners;
}

/*
	Private Host (CPU) functions to call kernel Device (GPU) functions
	==================================================================
*/

void CornerDetector::initConfig(Config& config)
{
	// Get parameters from configuration
	m_croppedRows			= (size_t)config.getIntValue(keyCroppedRows);
	m_gradientThreshold		= (byte)config.getIntValue(keyGradientThreshold);
	m_minDistancePixels		= (size_t)config.getIntValue(keyMinDistancePixels);
	m_minFoundCapillaries	= (size_t)config.getIntValue(keyMinFoundCapillaries);
}

void CornerDetector::writeCorners(const std::vector<ScoredCorner>& scoredCorners, const std::string& filenameLayer)
{
	std::ofstream fileLayer(filenameLayer);
	fileLayer << "Num,Row,Col,Gray level,Score" << std::endl;
	size_t number = 1;
	for (const ScoredCorner& scoredCorner : scoredCorners)
	{
		size_t row = mm2pixels(scoredCorner.y);
		size_t col = mm2pixels(scoredCorner.x);
		fileLayer <<
			number++ << "," <<
			row << "," <<
			col << "," <<
			(int)scoredCorner.grayLevel << "," <<
			std::setprecision(4) << scoredCorner.score << std::endl;
	}
	fileLayer.close();
}
