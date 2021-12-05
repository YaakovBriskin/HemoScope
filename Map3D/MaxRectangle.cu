#include "UtilsCUDA.h"
#include "MaxRectangle.h"

/*
	Kernel Device (GPU) variables and functions
	===========================================
*/

__device__ constexpr float PI() { return 3.14159265F; }

__global__ void resetRotatedCapillary(byte* d_dstMatrix, int dstRows, int dstCols)
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	if ((x >= dstCols) || (y >= dstRows))
	{
		return;
	}

	d_dstMatrix[y * dstCols + x] = LIGHT_GRAY;
}

__global__ void performCapillaryRotation(byte* d_srcMatrix, byte* d_dstMatrix,
	int srcRows, int srcCols, int dstRows, int dstCols,
	float srcCenterX, float srcCenterY, float dstCenterX, float dstCenterY, float angle)
{
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	if ((x >= srcCols) || (y >= srcRows))
	{
		return;
	}

	byte pixel = d_srcMatrix[y * srcCols + x];
	if (pixel != WHITE)
	{
		return;
	}

	float deltaX = srcCenterX - (float)x;
	float deltaY = srcCenterY - (float)y;
	float radius = sqrtf(deltaX * deltaX + deltaY * deltaY);
	float angleSrc = atan2f(deltaY, deltaX);
	float angleDst = angleSrc + angle + PI(); // axis Y is counter-directional to rows numeration
	size_t dstX = (size_t)((int)dstCenterX + (int)round(radius * cosf(angleDst)));
	size_t dstY = (size_t)((int)dstCenterY + (int)round(radius * sinf(angleDst)));
	d_dstMatrix[dstY * dstCols + dstX] = pixel;
}

/*
	Public Host (CPU) functions to call kernel Device (GPU) functions
	=================================================================
*/

MaxRectangle::MaxRectangle(ByteMatrix& byteMatrix, PixelPos start, size_t rows, size_t cols,
	const std::string& layerFolderName, size_t capillaryIndex)
{
	// Create and fill original rectangle
	m_originalCapillary = ByteMatrix(rows, cols);
	for (size_t row = 0; row < rows; row++)
	{
		for (size_t col = 0; col < cols; col++)
		{
			byte pixel = byteMatrix.get(start.pixelRow + row, start.pixelCol + col);
			m_originalCapillary.set(row, col, pixel);
		}
	}

#ifdef _DEBUG
	// Save image of original capillary
	std::string capillaryFilename = layerFolderName + "/Capillary" +
		std::to_string(capillaryIndex + 1) + ".bmp";
	cv::imwrite(capillaryFilename, m_originalCapillary.asCvMatU8());
#endif
	// Prepare byte matrices for rotated and dilated rectangle - large enough for any rotation
	size_t rotatedSize = 2 * std::max(rows, cols);
	m_rotatedCapillary = ByteMatrix(rotatedSize, rotatedSize);
	m_dilatedCapillary = ByteMatrix(rotatedSize, rotatedSize);

	// Calculate center of updated rectangle which is the same as center of original rectangle
	size_t centralRow = start.pixelRow + rows / 2;
	size_t centralCol = start.pixelCol + cols / 2;
	m_centerInImage = PixelPos(centralRow, centralCol);

	m_limitUp = 0;
	m_limitDn = 0;
	m_limitLf = 0;
	m_limitRt = 0;

	m_rowFrameInRotated = 0;
	m_colFrameInRotated = 0;
	m_foundAngleRadians = 0.0F;
	m_score = 0.0F;
}

std::vector<PixelPos> MaxRectangle::findRectangle(const std::string& layerFolderName, size_t capillaryIndex)
{
	std::vector<PixelPos> rotatedRectangle;
	size_t angleDegrees = 0;
	bool foundInscribedRectangle = false;
	for (; angleDegrees < 180; angleDegrees += 10)
	{
		rotateCapillary(angleDegrees);
		findCapillaryLimits();
		dilateRotatedCapillary();
		foundInscribedRectangle = findInscribedRectangle();
		if (foundInscribedRectangle)
		{
			break;
		}
	}

#ifdef _DEBUG
	// Save image of found rotated capillary
	std::string rotatedFilename = layerFolderName + "/Rotated" +
		std::to_string(capillaryIndex + 1) + ".bmp";
	cv::imwrite(rotatedFilename, m_rotatedCapillary.asCvMatU8());

	// Frame is marked only if inscribed rectangle is found
	markFrameInDilatedCapillary(foundInscribedRectangle);

	// Save image of found dilated capillary with frame
	std::string dilatedFilename = layerFolderName + "/Dilated" +
		std::to_string(capillaryIndex + 1) + ".bmp";
	cv::imwrite(dilatedFilename, m_dilatedCapillary.asCvMatU8());
#endif
	if (!foundInscribedRectangle)
	{
		return rotatedRectangle;
	}

	// Width map of the capillary along the longest axis of symmetry of the rectangle
	writeWidthMap(layerFolderName, capillaryIndex);

	// Convert found angle to radians and flip sign: rotated frame on fixed capillary
	m_foundAngleRadians = -deg2rad(angleDegrees);

	rotatedRectangle = getRotatedRectangle();
	return rotatedRectangle;
}

float MaxRectangle::getAngle()
{
	return m_foundAngleRadians;
}

float MaxRectangle::getScore()
{
	return 100.0F * (m_score - SCORE_THRESHOLD);
}

/*
	Private Host (CPU) functions to call kernel Device (GPU) functions
	==================================================================
*/

void MaxRectangle::rotateCapillary(size_t angleDegrees)
{
	// Convert given angle to radians
	float angle = deg2rad(angleDegrees);

	// Sizes of source and destination
	size_t rowsSrc = m_originalCapillary.rows();
	size_t colsSrc = m_originalCapillary.cols();
	size_t rowsDst = m_rotatedCapillary.rows();
	size_t colsDst = m_rotatedCapillary.cols();

	// Centers of source and destination
	size_t centerRowSrc = rowsSrc / 2;
	size_t centerColSrc = colsSrc / 2;
	size_t centerRowDst = rowsDst / 2;
	size_t centerColDst = colsDst / 2;

	// Allocate device memory buffers
	byte* d_srcBuffer = nullptr;
	checkCuda(cudaMalloc(&d_srcBuffer, rowsSrc * colsSrc));
	byte* d_dstBuffer = nullptr;
	checkCuda(cudaMalloc(&d_dstBuffer, rowsDst * colsDst));

	// Parameters to launch parallel threads
	dim3 blockSize(128, 1);
	dim3 numBlocksSrc(divideCeil((int)colsSrc, blockSize.x), divideCeil((int)rowsSrc, blockSize.y), 1);
	dim3 numBlocksDst(divideCeil((int)colsDst, blockSize.x), divideCeil((int)rowsDst, blockSize.y), 1);

	// Fill source device buffer by original capillary
	checkCuda(cudaMemcpy(d_srcBuffer, m_originalCapillary.getBuffer(), rowsSrc * colsSrc, cudaMemcpyHostToDevice));

	// Fill background of rotated capillary
	resetRotatedCapillary<<<numBlocksDst, blockSize>>>(d_dstBuffer, (int)rowsDst, (int)colsDst);

	// Calculate rotated capillary on GPU
	performCapillaryRotation<<<numBlocksSrc, blockSize>>>(d_srcBuffer, d_dstBuffer,
		(int)rowsSrc, (int)colsSrc, (int)rowsDst, (int)colsDst,
		(float)centerColSrc, (float)centerRowSrc, (float)centerColDst, (float)centerRowDst, angle);
	checkCuda(cudaDeviceSynchronize());

	// Get calculated rotated capillary from device memory and free it
	checkCuda(cudaMemcpy(m_rotatedCapillary.getBuffer(), d_dstBuffer, rowsDst * colsDst, cudaMemcpyDeviceToHost));
	checkCuda(cudaFree(d_dstBuffer));
}

void MaxRectangle::findCapillaryLimits()
{
	// Used to break nested loops
	bool found;

	// Find limit: Up
	found = false;
	for (size_t row = 0; (row < m_rotatedCapillary.rows()) && !found; row++)
	{
		for (size_t col = 0; (col < m_rotatedCapillary.cols()) && !found; col++)
		{
			byte pixel = m_rotatedCapillary.get(row, col);
			if (pixel == WHITE)
			{
				m_limitUp = row;
				found = true;
			}
		}
	}

	// Find limit: Rt
	found = false;
	for (size_t col = m_rotatedCapillary.cols() - 1; (col > 0) && !found; col--)
	{
		for (size_t row = 0; (row < m_rotatedCapillary.rows()) && !found; row++)
		{
			byte pixel = m_rotatedCapillary.get(row, col);
			if (pixel == WHITE)
			{
				m_limitRt = col;
				found = true;
			}
		}
	}

	// Find limit: Dn
	found = false;
	for (size_t row = m_rotatedCapillary.rows() - 1; (row > 0) && !found; row--)
	{
		for (size_t col = m_rotatedCapillary.cols() - 1; (col > 0) && !found; col--)
		{
			byte pixel = m_rotatedCapillary.get(row, col);
			if (pixel == WHITE)
			{
				m_limitDn = row;
				found = true;
			}
		}
	}

	// Find limit: Lf
	found = false;
	for (size_t col = 0; (col < m_rotatedCapillary.cols()) && !found; col++)
	{
		for (size_t row = m_rotatedCapillary.rows() - 1; (row > 0) && !found; row--)
		{
			byte pixel = m_rotatedCapillary.get(row, col);
			if (pixel == WHITE)
			{
				m_limitLf = col;
				found = true;
			}
		}
	}
}

void MaxRectangle::dilateRotatedCapillary()
{
	const size_t dilationKernelSize = 3;
	size_t halfKernelSize = dilationKernelSize / 2;
	size_t threshold = dilationKernelSize * dilationKernelSize / 2 - 1;

	// Reset previous dilation
	m_dilatedCapillary.clean();

	for (size_t row = m_limitUp; row <= m_limitDn; row++)
	{
		for (size_t col = m_limitLf; col <= m_limitRt; col++)
		{
			// Count pixels in kernel
			size_t numWhitePixelsInKernel = 0;
			for (size_t kernelRow = row - halfKernelSize; kernelRow <= row + halfKernelSize; kernelRow++)
			{
				for (size_t kernelCol = col - halfKernelSize; kernelCol <= col + halfKernelSize; kernelCol++)
				{
					if (m_rotatedCapillary.get(kernelRow, kernelCol) == WHITE)
					{
						numWhitePixelsInKernel++;
					}
				}
			}

			// Skip empty areas
			if (numWhitePixelsInKernel < threshold)
			{
				continue;
			}

			// Fill underfilled areas
			for (size_t kernelRow = row - halfKernelSize; kernelRow <= row + halfKernelSize; kernelRow++)
			{
				for (size_t kernelCol = col - halfKernelSize; kernelCol <= col + halfKernelSize; kernelCol++)
				{
					m_dilatedCapillary.set(kernelRow, kernelCol, WHITE);
				}
			}
		}
	}
}

bool MaxRectangle::findInscribedRectangle()
{
	for (size_t row = m_limitUp; row <= m_limitDn - FRAME_HEIGHT; row++)
	{
		for (size_t col = m_limitLf; col <= m_limitRt - FRAME_WIDTH; col++)
		{
			size_t numWhitePixelsInRectangle = 0;
			for (size_t frameRow = row; frameRow < row + FRAME_HEIGHT; frameRow++)
			{
				for (size_t frameCol = col; frameCol < col + FRAME_WIDTH; frameCol++)
				{
					if (m_dilatedCapillary.get(frameRow, frameCol) == WHITE)
					{
						numWhitePixelsInRectangle++;
					}
				}
			}

			float score = (float)numWhitePixelsInRectangle / FRAME_WIDTH / FRAME_HEIGHT;
			if (score > m_score)
			{
				m_rowFrameInRotated = row;
				m_colFrameInRotated = col;
				m_score = score;
			}
		}
	}

	return m_score >= SCORE_THRESHOLD;
}

void MaxRectangle::markFrameInDilatedCapillary(bool foundInscribedRectangle)
{
	if (!foundInscribedRectangle)
	{
		return;
	}

	for (size_t row = m_rowFrameInRotated; row <= m_rowFrameInRotated + FRAME_HEIGHT; row++)
	{
		m_dilatedCapillary.set(row, m_colFrameInRotated, BLACK);
		m_dilatedCapillary.set(row, m_colFrameInRotated + FRAME_WIDTH, BLACK);
	}

	for (size_t col = m_colFrameInRotated; col <= m_colFrameInRotated + FRAME_WIDTH; col++)
	{
		m_dilatedCapillary.set(m_rowFrameInRotated, col, BLACK);
		m_dilatedCapillary.set(m_rowFrameInRotated + FRAME_HEIGHT, col, BLACK);
	}
}

void MaxRectangle::writeWidthMap(const std::string& layerFolderName, size_t capillaryIndex)
{
	std::string filenameWidthMap = layerFolderName + "/WidthCapillary" +
		std::to_string(capillaryIndex + 1) + ".csv";
	std::ofstream fileWidthMap(filenameWidthMap);
	fileWidthMap << "Distance mm,Width mm" << std::endl;

	for (size_t row = m_limitUp; row <= m_limitDn; row++)
	{
		float distance = pixels2mm(row - m_limitUp);
		size_t widthPixels = 0;
		for (size_t col = m_limitLf; col <= m_limitRt; col++)
		{
			widthPixels += m_dilatedCapillary.get(row, col) == WHITE ? 1 : 0;
		}
		float width = pixels2mm(widthPixels);
		fileWidthMap <<
			std::setw(8) << distance << "," <<
			std::setw(8) << width << std::endl;
	}

	fileWidthMap.close();
}

std::vector<PixelPos> MaxRectangle::getRotatedRectangle()
{
	size_t centerRow = m_dilatedCapillary.rows() / 2;
	size_t centerCol = m_dilatedCapillary.cols() / 2;

	// Set vertices of original unrotated rectangle in predefined order
	std::vector<PixelPos> originalRectangle;
	originalRectangle.push_back(PixelPos(m_rowFrameInRotated, m_colFrameInRotated));
	originalRectangle.push_back(PixelPos(m_rowFrameInRotated, m_colFrameInRotated + FRAME_WIDTH));
	originalRectangle.push_back(PixelPos(m_rowFrameInRotated + FRAME_HEIGHT, m_colFrameInRotated + FRAME_WIDTH));
	originalRectangle.push_back(PixelPos(m_rowFrameInRotated + FRAME_HEIGHT, m_colFrameInRotated));

	std::vector<PixelPos> rotatedRectangle;
	for (const PixelPos& pixelPos : originalRectangle)
	{
		float deltaY = (float)centerRow - (float)pixelPos.pixelRow;
		float deltaX = (float)centerCol - (float)pixelPos.pixelCol;
		float radius = std::sqrtf(deltaX * deltaX + deltaY * deltaY);
		float angleSrc = std::atan2f(deltaY, deltaX);
		float angleDst = makeCentrosymmetric(angleSrc + m_foundAngleRadians); // axis Y is counter-directional to rows numeration
		size_t rowInImage = (size_t)((int)m_centerInImage.pixelRow + (int)std::round(radius * std::sinf(angleDst)));
		size_t colInImage = (size_t)((int)m_centerInImage.pixelCol + (int)std::round(radius * std::cosf(angleDst)));
		rotatedRectangle.push_back(PixelPos(rowInImage, colInImage));
	}

	return rotatedRectangle;
}
