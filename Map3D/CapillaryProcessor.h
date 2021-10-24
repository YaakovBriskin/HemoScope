#pragma once

#include "MaxRectangle.h"

class CapillaryInfo
{
public:
	size_t index;
	Point3D posApex;
	PixelPos pixelUp;
	PixelPos pixelDn;
	PixelPos pixelLf;
	PixelPos pixelRt;
	size_t pixelsCapillary;
	size_t energyCapillary;
	size_t pixelsSurroundings;
	size_t energySurroundings;

public:
	CapillaryInfo()
	{
		index = 0;
		pixelUp = PixelPos();
		pixelDn = PixelPos();
		pixelLf = PixelPos();
		pixelRt = PixelPos();
		pixelsCapillary = 0;
		energyCapillary = 0;
		pixelsSurroundings = 0;
		energySurroundings = 0;
	}
	
	void setPos(const ScoredCorner& scoredCorner)
	{
		posApex.x = scoredCorner.x;
		posApex.y = scoredCorner.y;
		posApex.z = scoredCorner.z;
	}
};

class CapillaryProcessor
{
public:
	CapillaryProcessor()
	{
		m_originalMatrix = ByteMatrix();
		m_processedMatrix = ByteMatrix();
	}

	void describeCapillaries(float startXmm, float startYmm, Map& map, const LayerInfo& layerInfo,
		const std::string& outFolderName)
	{
		// Filled description of capillaries for further statistics calculation
		std::vector<CapillaryInfo> capillariesInfo;

		// Create folder only for selected best layer with the corresponding name
		std::string layerFolderName = outFolderName + "/Layer" + std::to_string(layerInfo.layerIndex + 1);
		if (!std::filesystem::exists(std::filesystem::path(layerFolderName)))
		{
			bool result = std::filesystem::create_directory(std::filesystem::path(layerFolderName));
			if (!result)
			{
				throw std::exception(("Cannot create folder: " + layerFolderName).c_str());
			}
		}

		// Get selected best layer: desctiption and matrix
		Layer layer = map.getLayers()[layerInfo.layerIndex];
		m_originalMatrix = layer.matrix;
#ifdef _DEBUG
		cv::imwrite(layerFolderName + "/Original.bmp", m_originalMatrix.asCvMatU8());
#endif
		// Create and fill processed matrix of selected best layer
		m_processedMatrix = ByteMatrix(m_originalMatrix.rows(), m_originalMatrix.cols());
		performExcessFiltering(m_originalMatrix, m_processedMatrix); // members passed as parameters to enable filtering in chain
#ifdef _DEBUG
		cv::imwrite(layerFolderName + "/Processed.bmp", m_processedMatrix.asCvMatU8());
#endif
		size_t numOfDescribedCapillaries =
			std::min(layerInfo.capillaryApexes.size(), DESCRIBED_CAPILLARIES);

		std::cout << "Describing of capillaries started: " << numOfDescribedCapillaries << " capillaries" << std::endl << std::endl;

		// For each capillary in layer
		for (size_t capillaryIndex = 0; capillaryIndex < numOfDescribedCapillaries; capillaryIndex++)
		{
			ScoredCorner scoredCorner = layerInfo.capillaryApexes[capillaryIndex];

			CapillaryInfo capillaryInfo;
			capillaryInfo.index = capillaryIndex;
			capillaryInfo.setPos(scoredCorner);
			capillaryInfo.pixelUp = PixelPos(m_processedMatrix.rows(), m_processedMatrix.cols());
			capillaryInfo.pixelLf = PixelPos(m_processedMatrix.rows(), m_processedMatrix.cols());

			// Convert position of the corner from mm to pixels
			size_t cornerRow = mm2pixels(scoredCorner.y);
			size_t cornerCol = mm2pixels(scoredCorner.x);

			// Mark pixels of the capillary and update information
			performTraversalBFS(cornerRow, cornerCol, map, capillaryInfo);
			writeWidthMap(startXmm, startYmm, capillaryInfo, layerFolderName, capillaryIndex);
			capillariesInfo.push_back(capillaryInfo);

			std::cout << "Capillary " << capillaryIndex + 1 << ": " << capillaryInfo.pixelsCapillary << " pixels" << std::endl;
		}

		std::cout << std::endl << "Describing of capillaries completed" << std::endl << std::endl;
#ifdef _DEBUG
		cv::imwrite(layerFolderName + "/Marked.bmp", m_processedMatrix.asCvMatU8());
#endif
		collectSurroundings(capillariesInfo);
		calculateStatistics(startXmm, startYmm, capillariesInfo, layerFolderName);

#ifdef _DEBUG
		drawFrames(capillariesInfo);
		cv::imwrite(layerFolderName + "/Framed.bmp", m_originalMatrix.asCvMatU8());
#endif
	}

private:
	ByteMatrix m_originalMatrix;
	ByteMatrix m_processedMatrix;

private:
	void performGaussianBlur(ByteMatrix& src, ByteMatrix& dst)
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

	void performUniformSmoothing(ByteMatrix& src, ByteMatrix& dst)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		const size_t halfKkernelSize = FINE_SMOOTHING_KERNEL_SIZE >> 1;

		// For each row in the source matrix
		for (size_t row = 0; row < rows; row++)
		{
			// For each col in the source matrix
			for (size_t col = 0; col < cols; col++)
			{
				// Skip the margin of source - copy source pixels to destination
				if ((row < halfKkernelSize) || (row >= rows - halfKkernelSize) ||
					(col < halfKkernelSize) || (col >= cols - halfKkernelSize))
				{
					dst.set(row, col, src.get(row, col));
					continue;
				}

				// Calculate and set smoothed gray level
				unsigned int sum = 0;
				for (size_t kernelRow = row - halfKkernelSize; kernelRow <= row + halfKkernelSize; kernelRow++)
				{
					for (size_t kernelCol = col - halfKkernelSize; kernelCol <= col + halfKkernelSize; kernelCol++)
					{
						sum += src.get(kernelRow, kernelCol);
					}
				}
				float smoothed = (float)sum / FINE_SMOOTHING_KERNEL_SIZE / FINE_SMOOTHING_KERNEL_SIZE;
				byte smoothedPixel = (byte)std::roundf(smoothed);
				dst.set(row, col, smoothedPixel);
			}
		}
	}

	void performExcessFiltering(ByteMatrix& src, ByteMatrix& dst)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		const size_t halfKkernelSize = DEEP_SMOOTHING_KERNEL_SIZE >> 1;

		// For each row in the source matrix
		for (size_t row = 0; row < rows; row++)
		{
			// For each col in the source matrix
			for (size_t col = 0; col < cols; col++)
			{
				// Skip the margin of source with the zeroing of result
				if ((row < halfKkernelSize) || (row >= rows - halfKkernelSize) ||
					(col < halfKkernelSize) || (col >= cols - halfKkernelSize))
				{
					dst.set(row, col, 0);
					continue;
				}

				// Calculate excess over blurred
				unsigned int sum = 0;
				for (size_t kernelRow = row - halfKkernelSize; kernelRow <= row + halfKkernelSize; kernelRow++)
				{
					for (size_t kernelCol = col - halfKkernelSize; kernelCol <= col + halfKkernelSize; kernelCol++)
					{
						sum += src.get(kernelRow, kernelCol);
					}
				}
				float blurred = (float)sum / DEEP_SMOOTHING_KERNEL_SIZE / DEEP_SMOOTHING_KERNEL_SIZE;
				float excess = 2.0F * (src.get(row, col) / blurred - 0.75F);
				excess = std::fminf(std::fmaxf(0.0F, excess), 1.0F);
				byte normalizedExcess = (byte)std::roundf(255.0F * excess);
				dst.set(row, col, normalizedExcess);
			}
		}
	}

	/*
		Perform traversal of connected pixels in the area from given root: row and col.
		Pixels in the area assumed have valid gray level which means predefined interval.
		The area traversal is performed using the BFS (Breadth-First Search) algorithn.
		This algorithm is based on non-recursive iteration in loop with usage of queue.
		Recursive traversal with the DFS (Depth-First Search) leads to stack overflow.
	*/
	void performTraversalBFS(size_t row, size_t col, Map& map, CapillaryInfo& capillaryInfo)
	{
		// Define queue of pixel positions for traversal by BFS algorithn
		std::queue<PixelPos> pixels;

		// Root pixel is always with valid gray level
		PixelPos pixelRoot = PixelPos(row, col);
		pixels.push(pixelRoot);

		// The queue of pixels dynamically shrinks and grows on each cycle of the loop
		while (!pixels.empty())
		{
			// Take pixel from the front of queue and dequeue it as (already) processed
			PixelPos pixelPos = pixels.front();
			pixels.pop();

			// Skip already processed pixel - it occurs with the same directions in different order
			if (m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol) == 255)
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
			if ((pixelPos.pixelRow > DEEP_SMOOTHING_KERNEL_SIZE) &&
				isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow - 1, pixelPos.pixelCol)))
			{
				pixels.push(PixelPos(pixelPos.pixelRow - 1, pixelPos.pixelCol));
			}

			// Enqueue the pixel from (row + 1) if row is in limits and gray level of the pixel is valid
			if ((pixelPos.pixelRow < m_processedMatrix.rows() - DEEP_SMOOTHING_KERNEL_SIZE) &&
				isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow + 1, pixelPos.pixelCol)))
			{
				pixels.push(PixelPos(pixelPos.pixelRow + 1, pixelPos.pixelCol));
			}

			// Enqueue the pixel from (col - 1) if col is in limits and gray level of the pixel is valid
			if ((pixelPos.pixelCol > DEEP_SMOOTHING_KERNEL_SIZE) &&
				isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol - 1)))
			{
				pixels.push(PixelPos(pixelPos.pixelRow, pixelPos.pixelCol - 1));
			}

			// Enqueue the pixel from (col + 1) if col is in limits and gray level of the pixel is valid
			if ((pixelPos.pixelCol < m_processedMatrix.cols() - DEEP_SMOOTHING_KERNEL_SIZE) &&
				isValidGrayLevelProcessed(m_processedMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol + 1)))
			{
				pixels.push(PixelPos(pixelPos.pixelRow, pixelPos.pixelCol + 1));
			}
		}
	}

	void processPixel(const PixelPos& pixelPos, CapillaryInfo& capillaryInfo)
	{
		// Examine pixel row to be most up
		if (pixelPos.pixelRow < capillaryInfo.pixelUp.pixelRow)
		{
			capillaryInfo.pixelUp = pixelPos;
		}

		// Examine pixel row to be most down
		if (pixelPos.pixelRow > capillaryInfo.pixelDn.pixelRow)
		{
			capillaryInfo.pixelDn = pixelPos;
		}

		// Examine pixel col to be most left
		if (pixelPos.pixelCol < capillaryInfo.pixelLf.pixelCol)
		{
			capillaryInfo.pixelLf = pixelPos;
		}

		// Examine pixel col to be most right
		if (pixelPos.pixelCol > capillaryInfo.pixelRt.pixelCol)
		{
			capillaryInfo.pixelRt = pixelPos;
		}

		// Accumulate number of pixels and sum of gray levels in the capillary on original matrix
		capillaryInfo.pixelsCapillary++;
		capillaryInfo.energyCapillary += m_originalMatrix.get(pixelPos.pixelRow, pixelPos.pixelCol);

		// Mark the pixel as already processed
		m_processedMatrix.set(pixelPos.pixelRow, pixelPos.pixelCol, 255);
	}

	void collectSurroundings(std::vector<CapillaryInfo>& capillariesInfo)
	{
		const size_t surroundingPixels = 10;

		// Accumulate number of pixels and sum of gray levels in surrounding rectangles of each capillary
		for (CapillaryInfo& capillaryInfo : capillariesInfo)
		{
			if (capillaryInfo.pixelsCapillary < MIN_PIXELS_IN_CAPILLARY)
			{
				continue;
			}

			// Rectangle at up
			size_t rectUpUp = std::max((int)capillaryInfo.pixelUp.pixelRow - (int)surroundingPixels, 0);
			size_t rectUpDn = capillaryInfo.pixelUp.pixelRow;
			size_t rectUpLf = capillaryInfo.pixelLf.pixelCol;
			size_t rectUpRt = capillaryInfo.pixelRt.pixelCol;
			updateSurroundingData(capillaryInfo, rectUpUp, rectUpDn, rectUpLf, rectUpRt);

			// Rectangle at down
			size_t rectDnUp = capillaryInfo.pixelDn.pixelRow;
			size_t rectDnDn = std::min((int)capillaryInfo.pixelDn.pixelRow + (int)surroundingPixels, (int)m_processedMatrix.rows());
			size_t rectDnLf = capillaryInfo.pixelLf.pixelCol;
			size_t rectDnRt = capillaryInfo.pixelRt.pixelCol;
			updateSurroundingData(capillaryInfo, rectDnUp, rectDnDn, rectDnLf, rectDnRt);

			// Rectangle at left
			size_t rectLfUp = capillaryInfo.pixelUp.pixelRow;
			size_t rectLfDn = capillaryInfo.pixelDn.pixelRow;
			size_t rectLfLf = std::max((int)capillaryInfo.pixelLf.pixelCol - (int)surroundingPixels, 0);
			size_t rectLfRt = capillaryInfo.pixelLf.pixelCol;
			updateSurroundingData(capillaryInfo, rectLfUp, rectLfDn, rectLfLf, rectLfRt);

			// Rectangle at right
			size_t rectRtUp = capillaryInfo.pixelUp.pixelRow;
			size_t rectRtDn = capillaryInfo.pixelDn.pixelRow;
			size_t rectRtLf = capillaryInfo.pixelRt.pixelCol;
			size_t rectRtRt = std::min((int)capillaryInfo.pixelRt.pixelCol + (int)surroundingPixels, (int)m_processedMatrix.cols());
			updateSurroundingData(capillaryInfo, rectRtUp, rectRtDn, rectRtLf, rectRtRt);
		}
	}

	void updateSurroundingData(CapillaryInfo& capillaryInfo,
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

	void calculateStatistics(float startXmm, float startYmm, std::vector<CapillaryInfo>& capillariesInfo,
		const std::string& layerFolderName)
	{
		std::string filenameData = layerFolderName + "/Data.csv";
		std::ofstream fileData(filenameData);
		fileData << "Num,x (col),y (row),z,Angle rad,Contrast" << std::endl;

		for (const CapillaryInfo& capillaryInfo : capillariesInfo)
		{
			// Calculate angle
			float angle = 0.0F;
			if (capillaryInfo.pixelsCapillary >= MIN_PIXELS_IN_CAPILLARY)
			{
				size_t upToRt = capillaryInfo.pixelRt.pixelCol - capillaryInfo.pixelUp.pixelCol;
				size_t upToLf = capillaryInfo.pixelUp.pixelCol - capillaryInfo.pixelLf.pixelCol;
				if (upToRt < upToLf)
				{
					// Upper pixel is closer to the right side - clockwise (positive from Y-axis) rotation
					size_t diagonalUpRtX = (capillaryInfo.pixelUp.pixelCol + capillaryInfo.pixelRt.pixelCol) / 2;
					size_t diagonalUpRtY = (capillaryInfo.pixelUp.pixelRow + capillaryInfo.pixelRt.pixelRow) / 2;
					size_t diagonalDnLfX = (capillaryInfo.pixelDn.pixelCol + capillaryInfo.pixelLf.pixelCol) / 2;
					size_t diagonalDnLfY = (capillaryInfo.pixelDn.pixelRow + capillaryInfo.pixelLf.pixelRow) / 2;
					angle = std::atan2f((float)(diagonalDnLfY - diagonalUpRtY), (float)(diagonalUpRtX - diagonalDnLfX));
				}
				else
				{
					// Upper pixel is closer to the left side - counter clockwise (negative from Y-axis) rotation
					size_t diagonalUpLfX = (capillaryInfo.pixelUp.pixelCol + capillaryInfo.pixelLf.pixelCol) / 2;
					size_t diagonalUpLfY = (capillaryInfo.pixelUp.pixelRow + capillaryInfo.pixelLf.pixelRow) / 2;
					size_t diagonalDnRtX = (capillaryInfo.pixelDn.pixelCol + capillaryInfo.pixelRt.pixelCol) / 2;
					size_t diagonalDnRtY = (capillaryInfo.pixelDn.pixelRow + capillaryInfo.pixelRt.pixelRow) / 2;
					angle = -std::atan2f((float)(diagonalDnRtY - diagonalUpLfY), (float)(diagonalDnRtX - diagonalUpLfX));
				}
			}

			// Calculate contrast
			byte contrast = 0;
			if (capillaryInfo.pixelsCapillary >= MIN_PIXELS_IN_CAPILLARY)
			{
				byte avgGrayLevelCapillary =
					(byte)std::roundf((float)capillaryInfo.energyCapillary / capillaryInfo.pixelsCapillary);
				byte avgGrayLevelSurroundings =
					(byte)std::roundf((float)capillaryInfo.energySurroundings / capillaryInfo.pixelsSurroundings);
				contrast = avgGrayLevelSurroundings - avgGrayLevelCapillary;
			}

			fileData <<
				capillaryInfo.index + 1 << "," <<
				capillaryInfo.posApex.x + startXmm << " (" << mm2pixels(capillaryInfo.posApex.x) << ")," <<
				capillaryInfo.posApex.y + startYmm << " (" << mm2pixels(capillaryInfo.posApex.y) << ")," <<
				capillaryInfo.posApex.z << "," <<
				angle << "," <<
				(int)contrast << std::endl;
		}

		fileData.close();
	}

	void writeWidthMap(float startXmm, float startYmm, const CapillaryInfo& capillaryInfo,
		const std::string& layerFolderName, size_t capillaryIndex)
	{
		std::string filenameWidthMap = layerFolderName + "/WidthMapCapillary" + std::to_string(capillaryIndex + 1) + ".csv";
		std::ofstream fileWidthMap(filenameWidthMap);

		size_t width  = capillaryInfo.pixelRt.pixelCol - capillaryInfo.pixelLf.pixelCol;
		size_t height = capillaryInfo.pixelDn.pixelRow - capillaryInfo.pixelUp.pixelRow;

		if (width < height)
		{
			fileWidthMap << "y,Width" << std::endl;
			for (size_t row = capillaryInfo.pixelUp.pixelRow; row <= capillaryInfo.pixelDn.pixelRow; row++)
			{
				size_t pixelCount = 0;
				for (size_t col = capillaryInfo.pixelLf.pixelCol; col <= capillaryInfo.pixelRt.pixelCol; col++)
				{
					if (m_processedMatrix.get(row, col) == 255)
					{
						pixelCount++;
					}
				}
				fileWidthMap << startYmm + pixels2mm(row) << "," << pixels2mm(pixelCount) << std::endl;
			}
		}
		else
		{
			fileWidthMap << "x,Width" << std::endl;
			for (size_t col = capillaryInfo.pixelLf.pixelCol; col <= capillaryInfo.pixelRt.pixelCol; col++)
			{
				size_t pixelCount = 0;
				for (size_t row = capillaryInfo.pixelUp.pixelRow; row <= capillaryInfo.pixelDn.pixelRow; row++)
				{
					if (m_processedMatrix.get(row, col) == 255)
					{
						pixelCount++;
					}
				}
				fileWidthMap << startXmm + pixels2mm(col) << "," << pixels2mm(pixelCount) << std::endl;
			}
		}

		fileWidthMap.close();
	}

	void drawFrames(std::vector<CapillaryInfo>& capillariesInfo)
	{
		for (const CapillaryInfo& capillaryInfo : capillariesInfo)
		{
			size_t frameUp = capillaryInfo.pixelUp.pixelRow;
			size_t frameDn = capillaryInfo.pixelDn.pixelRow;
			size_t frameLf = capillaryInfo.pixelLf.pixelCol;
			size_t frameRt = capillaryInfo.pixelRt.pixelCol;

			for (size_t row = frameUp; row <= frameDn; row++)
			{
				m_originalMatrix.set(row, frameLf, 255);
				m_originalMatrix.set(row, frameRt, 255);
			}

			for (size_t col = frameLf; col <= frameRt; col++)
			{
				m_originalMatrix.set(frameUp, col, 255);
				m_originalMatrix.set(frameDn, col, 255);
			}
		}
	}
};
