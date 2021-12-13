#include "Utils.h"
#include "UtilsCUDA.h"
#include "CapillaryProcessor.h"

/*
	Kernel Device (GPU) variables and functions
	===========================================
*/

__global__ void applyHPF(byte* d_srcMatrix, byte* d_dstMatrix, int rows, int cols,
	size_t deepSmoothingKernelSize)
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	if ((x >= cols) || (y >= rows))
	{
		return;
	}

	const size_t halfKernelSize = deepSmoothingKernelSize / 2;

	// Skip margins with zeroing of result
	if ((x < halfKernelSize) ||
		(x >= cols - halfKernelSize) ||
		(y < halfKernelSize) ||
		(y >= rows - halfKernelSize))
	{
		d_dstMatrix[y * cols + x] = 0;
		return;
	}

	// Calculate excess over blurred
	unsigned int sum = 0;
	for (size_t kernelRow = y - halfKernelSize; kernelRow <= y + halfKernelSize; kernelRow++)
	{
		for (size_t kernelCol = x - halfKernelSize; kernelCol <= x + halfKernelSize; kernelCol++)
		{
			sum += d_srcMatrix[kernelRow * cols + kernelCol];
		}
	}
	float blurred = (float)sum / deepSmoothingKernelSize / deepSmoothingKernelSize;
	float excess = 2.0F * (d_srcMatrix[y * cols + x] / blurred - 0.75F);

	if (excess < 0.0F)
	{
		excess = 0.0F;
	}
	if (excess > 1.0F)
	{
		excess = 1.0F;
	}

	byte normalizedExcess = (byte)(WHITE * excess + 0.5F);
	d_dstMatrix[y * cols + x] = normalizedExcess;
}

/*
	Public Host (CPU) functions to call kernel Device (GPU) functions
	=================================================================
*/

CapillaryProcessor::CapillaryProcessor()
{
	m_fineSmoothingKernelSize = 0;
	m_deepSmoothingKernelSize = 0;
	m_numDescribedCappilaries = 0;
	m_minPixelsInCappilary = 0;
	m_surroundingPixels = 0;

	m_originalMatrix = ByteMatrix();
	m_processedMatrix = ByteMatrix();
	m_layerIndex = 0;
}

void CapillaryProcessor::init(Config& config)
{
	initConfig(config);
}

void CapillaryProcessor::describeCapillaries(Map& map, LayerInfo& layerInfo, const std::string& outputFolderName)
{
	m_layerIndex = layerInfo.layerIndex;

	// Reset scores given by corner detection - new scores will be given by pixels in frame
	layerInfo.maxScore = 0.0F;
	layerInfo.sumScore = 0.0F;

	// Create folder for layer with the corresponding name
	std::string layerFolderName = "Layer" + std::to_string(layerInfo.layerIndex + 1);
	createFoldersIfNeed(outputFolderName, layerFolderName);

	// Get pixel matrix of current layer from the map
	Layer layer = map.getLayers()[layerInfo.layerIndex];
	m_originalMatrix = layer.matrix;
#ifdef _DEBUG
	cv::imwrite(outputFolderName + "/" + layerFolderName + "/Original.bmp", m_originalMatrix.asCvMatU8());
#endif
	// Create and fill processed matrix of current layer
	m_processedMatrix = ByteMatrix(m_originalMatrix.rows(), m_originalMatrix.cols());

	// Members passed as parameters to support filtering in chain
	performExcessFiltering(m_originalMatrix, m_processedMatrix);
#ifdef _DEBUG
	cv::imwrite(outputFolderName + "/" + layerFolderName + "/Processed.bmp", m_processedMatrix.asCvMatU8());
#endif
	size_t numOfDescribedCapillaries = layerInfo.capillaryApexes.size();

	std::cout << "Layer " << m_layerIndex + 1 << " - describing of capillaries started: " <<
		numOfDescribedCapillaries << " capillaries" << std::endl;
	m_timer.start();

	// For each capillary in the layer: calculate and collect information about the capillary
	for (size_t capillaryIndex = 0; capillaryIndex < numOfDescribedCapillaries; capillaryIndex++)
	{
		// Get coordinates of detected point in the capillary
		ScoredCorner scoredCorner = layerInfo.capillaryApexes[capillaryIndex];

		// Set initial information of the capillary
		CapillaryInfo capillaryInfo;
		capillaryInfo.index = capillaryIndex;
		capillaryInfo.setPos(scoredCorner);

		// Convert position of the corner from mm to pixels
		size_t cornerRow = mm2pixels(scoredCorner.y);
		size_t cornerCol = mm2pixels(scoredCorner.x);

		// Mark pixels of the capillary and update information
		performTraversalBFS(cornerRow, cornerCol, map, capillaryInfo);

		// Skip too sparse capillary
		if (capillaryInfo.pixelsCapillary < m_minPixelsInCappilary)
		{
			continue;
		}

		std::string info = "Capillary " + std::to_string(capillaryIndex + 1) + ": " +
			std::to_string(capillaryInfo.pixelsCapillary) + " pixels";

		// Skip too low or narrow capillary
		size_t capillaryRows = capillaryInfo.limitDn - capillaryInfo.limitUp + 1;
		size_t capillaryCols = capillaryInfo.limitRt - capillaryInfo.limitLf + 1;
		if ((capillaryRows < FRAME_HEIGHT) || (capillaryCols < FRAME_WIDTH))
		{
			info += " - too small";
			std::cout << info << std::endl;
			continue;
		}

		// Instance to find inscribed rotated frame
		MaxRectangle maxRectangleFinder(m_processedMatrix,
			PixelPos(capillaryInfo.limitUp, capillaryInfo.limitLf),
			capillaryRows, capillaryCols, outputFolderName + "/" + layerFolderName, capillaryIndex);

		// Rotated frame - is empty if cannot find rectangle with score over the threshold
		std::vector<PixelPos> rotatedRectangle =
			maxRectangleFinder.findRectangle(outputFolderName + "/" + layerFolderName, capillaryIndex);

		// Skip capillary with score lower than the threshold for which no frame was found
		if (rotatedRectangle.empty())
		{
			info += " - score is lower than threshold";
			std::cout << info << std::endl;
			continue;
		}

		// Angle of frame rotation
		capillaryInfo.angle = maxRectangleFinder.getAngle();

		// Score indicated percentage of marked pixels in the frame
		capillaryInfo.score = maxRectangleFinder.getScore();

		info += " - score = " + toString(capillaryInfo.score, 1);
		std::cout << info << std::endl;

		// Update description of capillaries for further statistics and score calculation
		layerInfo.capillariesInfo.push_back(capillaryInfo);
#ifdef _DEBUG
		drawRotatedFrame(rotatedRectangle);
#endif
	}

	m_timer.end();
	std::cout << "Layer " << m_layerIndex + 1 <<
		" - describing of capillaries completed in " <<
		m_timer.getDurationMilliseconds() << " ms" << std::endl << std::endl;
#ifdef _DEBUG
	cv::imwrite(outputFolderName + "/" + layerFolderName + "/Marked.bmp", m_processedMatrix.asCvMatU8());
#endif
	if (layerInfo.capillariesInfo.empty())
	{
		std::cout << "Layer " << m_layerIndex + 1 <<
			" - no capillaries found to hold FOV frame" << std::endl << std::endl;
		return;
	}

	float startXmm = map.getStartXmm();
	float startYmm = map.getStartYmm();
	collectSurroundings(layerInfo.capillariesInfo);
	trimAndSetLayerScores(layerInfo, startXmm, startYmm,
		layerInfo.capillariesInfo, outputFolderName + "/" + layerFolderName);
#ifdef _DEBUG
	cv::imwrite(outputFolderName + "/" + layerFolderName + "/Framed.bmp", m_originalMatrix.asCvMatU8());
#endif
}

/*
	Private Host (CPU) functions to call kernel Device (GPU) functions
	==================================================================
*/

void CapillaryProcessor::initConfig(Config& config)
{
	// Get parameters from configuration
	m_fineSmoothingKernelSize	= (size_t)config.getIntValue(keyFineSmoothingKernelSize);
	m_deepSmoothingKernelSize	= (size_t)config.getIntValue(keyDeepSmoothingKernelSize);
	m_numDescribedCappilaries	= (size_t)config.getIntValue(keyNumDescribedCappilaries);
	m_minPixelsInCappilary		= (size_t)config.getIntValue(keyMinPixelsInCappilary);
	m_surroundingPixels			= (size_t)config.getIntValue(keySurroundingPixels);
}

void CapillaryProcessor::performGaussianBlur(ByteMatrix& src, ByteMatrix& dst)
{
	size_t rows = src.rows();
	size_t cols = src.cols();

	// For each row in the source matrix
	for (size_t row = 0; row < rows; row++)
	{
		// For each col in the source matrix
		for (size_t col = 0; col < cols; col++)
		{
			// Skip the margin of source - copy source pixels to destination
			if ((row == 0) || (row == rows - 1) || (col == 0) || (col == cols - 1))
			{
				dst.set(row, col, src.get(row, col));
				continue;
			}

			// Calculate Gaussian blur
			unsigned short sum = 4 * src.get(row, col) +
				2 * (src.get(row, col - 1) + src.get(row, col + 1) + src.get(row - 1, col) + src.get(row + 1, col)) +
				src.get(row - 1, col - 1) + src.get(row - 1, col + 1) + src.get(row + 1, col - 1) + src.get(row + 1, col + 1);
			byte blurredPixel = (byte)std::roundf((float)sum / 16);
			dst.set(row, col, blurredPixel);
		}
	}
}

void CapillaryProcessor::performUniformSmoothing(ByteMatrix& src, ByteMatrix& dst)
{
	size_t rows = src.rows();
	size_t cols = src.cols();

	const size_t halfKernelSize = m_fineSmoothingKernelSize / 2;

	// For each row in the source matrix
	for (size_t row = 0; row < rows; row++)
	{
		// For each col in the source matrix
		for (size_t col = 0; col < cols; col++)
		{
			// Skip the margin of source - copy source pixels to destination
			if ((row < halfKernelSize) || (row >= rows - halfKernelSize) ||
				(col < halfKernelSize) || (col >= cols - halfKernelSize))
			{
				dst.set(row, col, src.get(row, col));
				continue;
			}

			// Calculate and set smoothed gray level
			unsigned int sum = 0;
			for (size_t kernelRow = row - halfKernelSize; kernelRow <= row + halfKernelSize; kernelRow++)
			{
				for (size_t kernelCol = col - halfKernelSize; kernelCol <= col + halfKernelSize; kernelCol++)
				{
					sum += src.get(kernelRow, kernelCol);
				}
			}
			float smoothed = (float)sum / m_fineSmoothingKernelSize / m_fineSmoothingKernelSize;
			byte smoothedPixel = (byte)std::roundf(smoothed);
			dst.set(row, col, smoothedPixel);
		}
	}
}

void CapillaryProcessor::performExcessFiltering(ByteMatrix& src, ByteMatrix& dst)
{
	size_t rows = src.rows();
	size_t cols = src.cols();

	// Parameters to launch parallel threads
	dim3 blockSize(128, 1);
	dim3 numBlocks(divideCeil((int)cols, blockSize.x), divideCeil((int)rows, blockSize.y), 1);

	std::cout << "Layer " << m_layerIndex + 1 <<
		" - applying of excess HPF started" << std::endl;
	m_timer.start();

	// Allocate device memory buffers and fill source device buffer by processed matrix
	byte* d_srcBuffer = nullptr;
	byte* d_dstBuffer = nullptr;
	checkCuda(cudaMalloc(&d_srcBuffer, rows * cols));
	checkCuda(cudaMalloc(&d_dstBuffer, rows * cols));
	checkCuda(cudaMemcpy(d_srcBuffer, src.getBuffer(), rows * cols, cudaMemcpyHostToDevice));

	// Calculate excess on GPU
	applyHPF<<<numBlocks, blockSize>>>(d_srcBuffer, d_dstBuffer, (int)rows, (int)cols,
		m_deepSmoothingKernelSize);
	checkCuda(cudaDeviceSynchronize());

	// Get calculated excess from device memory and free it
	checkCuda(cudaMemcpy(dst.getBuffer(), d_dstBuffer, rows * cols, cudaMemcpyDeviceToHost));
	checkCuda(cudaFree(d_srcBuffer));
	checkCuda(cudaFree(d_dstBuffer));

	m_timer.end();
	std::cout << "Layer " << m_layerIndex + 1 <<
		" - applying of excess HPF completed in " <<
		m_timer.getDurationMilliseconds() << " ms" << std::endl;
}

/*
	Perform traversal of connected pixels in the area from given root: row and col.
	Pixels in the area assumed have valid gray level which means predefined interval.
	The area traversal is performed using the BFS (Breadth-First Search) algorithn.
	This algorithm is based on non-recursive iteration in loop with usage of queue.
	Recursive traversal with the DFS (Depth-First Search) leads to stack overflow.
*/
void CapillaryProcessor::performTraversalBFS(size_t row, size_t col, Map& map, CapillaryInfo& capillaryInfo)
{
	// Define queue of pixel positions for traversal by BFS algorithn
	std::queue<PixelPos> pixels;

	// Root pixel is always with valid gray level
	PixelPos pixelRoot = PixelPos(row, col);
	pixels.push(pixelRoot);

	// Init upper and left limits for further minimization
	capillaryInfo.limitUp = m_processedMatrix.rows();
	capillaryInfo.limitLf = m_processedMatrix.cols();

	// The queue of pixels dynamically shrinks and grows on each cycle of the loop
	while (!pixels.empty())
	{
		// Take pixel from the front of queue and dequeue it as (already) processed
		PixelPos pixelPos = pixels.front();
		pixels.pop();

		// Skip already processed pixel - it occurs with the same directions in different order
		if (m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol) == WHITE)
		{
			continue;
		}

		// Do not process pixels on seams but continue the traversal
		if (map.isOnSeam(pixelPos.pixelRow, true) || map.isOnSeam(pixelPos.pixelCol, false))
		{
			continue;
		}

		// Process the pixel and mark as already processed
		processPixel(pixelPos, capillaryInfo);

		// Enqueue the pixel from (row - 1) if row is in limits and gray level of the pixel is valid
		if ((pixelPos.pixelRow > m_deepSmoothingKernelSize) &&
			isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow - 1, pixelPos.pixelCol)))
		{
			pixels.push(PixelPos(pixelPos.pixelRow - 1, pixelPos.pixelCol));
		}

		// Enqueue the pixel from (row + 1) if row is in limits and gray level of the pixel is valid
		if ((pixelPos.pixelRow < m_processedMatrix.rows() - m_deepSmoothingKernelSize) &&
			isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow + 1, pixelPos.pixelCol)))
		{
			pixels.push(PixelPos(pixelPos.pixelRow + 1, pixelPos.pixelCol));
		}

		// Enqueue the pixel from (col - 1) if col is in limits and gray level of the pixel is valid
		if ((pixelPos.pixelCol > m_deepSmoothingKernelSize) &&
			isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol - 1)))
		{
			pixels.push(PixelPos(pixelPos.pixelRow, pixelPos.pixelCol - 1));
		}

		// Enqueue the pixel from (col + 1) if col is in limits and gray level of the pixel is valid
		if ((pixelPos.pixelCol < m_processedMatrix.cols() - m_deepSmoothingKernelSize) &&
			isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol + 1)))
		{
			pixels.push(PixelPos(pixelPos.pixelRow, pixelPos.pixelCol + 1));
		}
	}
}

void CapillaryProcessor::processPixel(const PixelPos& pixelPos, CapillaryInfo& capillaryInfo)
{
	// Update limits if processed pixel exceeds existing
	capillaryInfo.limitUp = std::min(capillaryInfo.limitUp, pixelPos.pixelRow);
	capillaryInfo.limitDn = std::max(capillaryInfo.limitDn, pixelPos.pixelRow);
	capillaryInfo.limitLf = std::min(capillaryInfo.limitLf, pixelPos.pixelCol);
	capillaryInfo.limitRt = std::max(capillaryInfo.limitRt, pixelPos.pixelCol);

	// Accumulate number of pixels and sum of gray levels in the capillary on original matrix
	capillaryInfo.pixelsCapillary++;
	capillaryInfo.energyCapillary += m_originalMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol);

	// Mark the pixel as already processed
	m_processedMatrix.set(pixelPos.pixelRow, pixelPos.pixelCol, WHITE);
}

void CapillaryProcessor::collectSurroundings(std::vector<CapillaryInfo>& capillariesInfo)
{
	// Accumulate number of pixels and sum of gray levels in surrounding rectangles of each capillary
	for (CapillaryInfo& capillaryInfo : capillariesInfo)
	{
		if (capillaryInfo.pixelsCapillary < m_minPixelsInCappilary)
		{
			continue;
		}

		// Rectangle at up
		size_t rectUpUp = std::max((int)capillaryInfo.limitUp - (int)m_surroundingPixels, 0);
		size_t rectUpDn = capillaryInfo.limitUp;
		size_t rectUpLf = capillaryInfo.limitLf;
		size_t rectUpRt = capillaryInfo.limitRt;
		updateSurroundingData(capillaryInfo, rectUpUp, rectUpDn, rectUpLf, rectUpRt);

		// Rectangle at down
		size_t rectDnUp = capillaryInfo.limitDn;
		size_t rectDnDn = std::min((int)capillaryInfo.limitDn + (int)m_surroundingPixels, (int)m_processedMatrix.rows());
		size_t rectDnLf = capillaryInfo.limitLf;
		size_t rectDnRt = capillaryInfo.limitRt;
		updateSurroundingData(capillaryInfo, rectDnUp, rectDnDn, rectDnLf, rectDnRt);

		// Rectangle at left
		size_t rectLfUp = capillaryInfo.limitUp;
		size_t rectLfDn = capillaryInfo.limitDn;
		size_t rectLfLf = std::max((int)capillaryInfo.limitLf - (int)m_surroundingPixels, 0);
		size_t rectLfRt = capillaryInfo.limitLf;
		updateSurroundingData(capillaryInfo, rectLfUp, rectLfDn, rectLfLf, rectLfRt);

		// Rectangle at right
		size_t rectRtUp = capillaryInfo.limitUp;
		size_t rectRtDn = capillaryInfo.limitDn;
		size_t rectRtLf = capillaryInfo.limitRt;
		size_t rectRtRt = std::min((int)capillaryInfo.limitRt + (int)m_surroundingPixels, (int)m_processedMatrix.cols());
		updateSurroundingData(capillaryInfo, rectRtUp, rectRtDn, rectRtLf, rectRtRt);
	}
}

void CapillaryProcessor::updateSurroundingData(CapillaryInfo& capillaryInfo,
	size_t rectUp, size_t rectDn, size_t rectLf, size_t rectRt)
{
	capillaryInfo.pixelsSurroundings += (rectDn - rectUp) * (rectRt - rectLf);
	for (size_t row = rectUp; row < rectDn; row++)
	{
		for (size_t col = rectLf; col < rectRt; col++)
		{
			capillaryInfo.energySurroundings += m_processedMatrix.get(row, col);
		}
	}
}

void CapillaryProcessor::trimAndSetLayerScores(LayerInfo& layerInfo, float startXmm, float startYmm,
	std::vector<CapillaryInfo>& capillariesInfo, const std::string& layerFolderName)
{
	// Sort found capillaries by score in descending order
	std::sort(capillariesInfo.begin(), capillariesInfo.end(), [](CapillaryInfo capillaryL, CapillaryInfo capillaryR) {
		return capillaryL.score > capillaryR.score;
		});
	layerInfo.maxScore = capillariesInfo.begin()->score;

	// Keep only predefined number of capillaries with best scores
	if (capillariesInfo.size() > m_numDescribedCappilaries)
	{
		capillariesInfo.erase(capillariesInfo.begin() + m_numDescribedCappilaries, capillariesInfo.end());
	}

	std::string filenameData = layerFolderName + "/Data.csv";
	std::ofstream fileData(filenameData);
	fileData << "Num,x (col),y (row),z,Angle rad,Contrast,Score" << std::endl;

	// For each capillary among predefined number of capillaries with best scores
	for (const CapillaryInfo& capillaryInfo : capillariesInfo)
	{
		// Calculate contrast
		byte contrast = 0;
		if (capillaryInfo.pixelsCapillary >= m_minPixelsInCappilary)
		{
			byte avgGrayLevelCapillary =
				(byte)std::roundf((float)capillaryInfo.energyCapillary / capillaryInfo.pixelsCapillary);
			byte avgGrayLevelSurroundings =
				(byte)std::roundf((float)capillaryInfo.energySurroundings / capillaryInfo.pixelsSurroundings);
			contrast = avgGrayLevelSurroundings - avgGrayLevelCapillary;
		}

		fileData <<
			capillaryInfo.index + 1 << "," <<
			std::setprecision(4) << capillaryInfo.posApex.x + startXmm << " (" << mm2pixels(capillaryInfo.posApex.x) << ")," <<
			std::setprecision(4) << capillaryInfo.posApex.y + startYmm << " (" << mm2pixels(capillaryInfo.posApex.y) << ")," <<
			std::setprecision(4) << capillaryInfo.posApex.z << "," <<
			std::setprecision(2) << capillaryInfo.angle << "," <<
			(int)contrast << "," <<
			std::setprecision(2) << capillaryInfo.score << std::endl;

		layerInfo.sumScore += capillaryInfo.score;
	}

	fileData.close();
}

void CapillaryProcessor::drawRotatedFrame(const std::vector<PixelPos>& rotatedFrame)
{
	drawLine(rotatedFrame[0], rotatedFrame[1]);
	drawLine(rotatedFrame[1], rotatedFrame[2]);
	drawLine(rotatedFrame[2], rotatedFrame[3]);
	drawLine(rotatedFrame[3], rotatedFrame[0]);
}

void CapillaryProcessor::drawLine(PixelPos pixelA, PixelPos pixelB)
{
	PixelPos pixelBegin;
	PixelPos pixelEnd;

	// Check whether loop iteration is performed along X or Y
	if (std::abs((int)pixelA.pixelCol - (int)pixelB.pixelCol) >=
		std::abs((int)pixelA.pixelRow - (int)pixelB.pixelRow))
	{
		if (pixelA.pixelCol < pixelB.pixelCol)
		{
			pixelBegin = pixelA;
			pixelEnd = pixelB;
		}
		else
		{
			pixelBegin = pixelB;
			pixelEnd = pixelA;
		}
		float slope = ((float)pixelEnd.pixelRow - (float)pixelBegin.pixelRow) /
			((float)pixelEnd.pixelCol - (float)pixelBegin.pixelCol);
		for (size_t col = pixelBegin.pixelCol; col <= pixelEnd.pixelCol; col++)
		{
			size_t row = pixelBegin.pixelRow +
				(size_t)std::roundf(slope * (col - pixelBegin.pixelCol));
			m_originalMatrix.set(row, col, WHITE);
		}
	}
	else
	{
		if (pixelA.pixelRow < pixelB.pixelRow)
		{
			pixelBegin = pixelA;
			pixelEnd = pixelB;
		}
		else
		{
			pixelBegin = pixelB;
			pixelEnd = pixelA;
		}
		float slope = ((float)pixelEnd.pixelCol - (float)pixelBegin.pixelCol) /
			((float)pixelEnd.pixelRow - (float)pixelBegin.pixelRow);
		for (size_t row = pixelBegin.pixelRow; row <= pixelEnd.pixelRow; row++)
		{
			size_t col = pixelBegin.pixelCol +
				(size_t)std::roundf(slope * (row - pixelBegin.pixelRow));
			m_originalMatrix.set(row, col, WHITE);
		}
	}
}
