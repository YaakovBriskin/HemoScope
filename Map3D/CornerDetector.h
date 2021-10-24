#pragma once

#include "Utils.h"

class CornerDetector
{
public:
	CornerDetector()
	{
		m_z = 0.0F;
		m_croppedRows = 400;
		m_gradientThreshold = 55;
		m_sensitivity = 0.05F;
		m_qualityLevel = 1200.0F;
		m_minDistancePixels = 200;
	}

	~CornerDetector() = default;

	void setLayerPosition(float z)
	{
		m_z = z;
	}

	/*
		Apply Gx and Gy Sobel kernels and find average gradient values over predefined threshold
	*/
	std::vector<ScoredCorner> getCornersSobel(Map& map, ByteMatrix& matrix,
		const std::string& capillariesFolderName, size_t layerIndex)
	{
		const float minDistanceBetweenCorners = 7.0F;

		// Number of pixels around the central pixel for valid kernel odd sizes: 3, 5, 7
		size_t halfKernelSize = CORNER_DETECTION_KERNEL_SIZE >> 1;

		std::vector<ScoredCorner> scoredCorners;

		ByteMatrix gradient = ByteMatrix(matrix.rows(), matrix.cols());
		applySobelFilter(matrix, gradient);

		for (size_t row = halfKernelSize; row < matrix.rows() - m_croppedRows - halfKernelSize; row++)
		{
			for (size_t col = halfKernelSize; col < matrix.cols() - halfKernelSize; col++)
			{
				// Skip possible false-positive corners on seams
				if (map.isOnSeam(row, true) || map.isOnSeam(col, false))
				{
					continue;
				}

				// Skip unexpected gray levels - mainly on flares
				byte grayLevel = matrix.get(row, col);
				if (!isValidGrayLevelRaw(grayLevel))
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

#ifdef _DEBUG
		std::string filenameGradient = capillariesFolderName + "/Gradient" + std::to_string(layerIndex + 1) + ".bmp";
		cv::imwrite(filenameGradient, gradient.asCvMatU8());
		std::string filenameLayer = capillariesFolderName + "/Layer" + std::to_string(layerIndex + 1) + ".csv";
		storeCorners(scoredCorners, filenameLayer);
#endif
		return scoredCorners;
	}

private:
	float m_z;

	// Limit number of rows for the search of corners
	size_t m_croppedRows;

	// Detect corners on the gradient matrix after applocation of Sobel filter
	byte m_gradientThreshold;

	// Adjustable coefficient of Harris algorithm which values can be 0.04 - 0.06 (up to 0.15 is legal)
	float m_sensitivity;

	// Threshold to accept or reject the calculated response
	float m_qualityLevel;

	// Minimal allowed distance between detected corners
	size_t m_minDistancePixels;

private:
	void applySobelFilter(ByteMatrix& src, ByteMatrix& dst)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		ByteMatrix bufferGx = ByteMatrix(rows, cols);
		ByteMatrix bufferGy = ByteMatrix(rows, cols);

		short kernelGx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
		short kernelGy[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

		applyKernel(src, bufferGx, (short*)kernelGx);
		applyKernel(src, bufferGy, (short*)kernelGy);

		// For each row in the source matrix
		for (size_t row = 0; row < rows; row++)
		{
			// For each col in the source matrix
			for (size_t col = 0; col < cols; col++)
			{
				// Magnitude (calculated as norm 1) must be trimmed before setting
				byte trimmedResult =
					(byte)std::min((short)bufferGx.get(row, col) + (short)bufferGy.get(row, col), 255);
				dst.set(row, col, trimmedResult);
			}
		}
	}

	void applyKernel(ByteMatrix& src, ByteMatrix& dst, short* kernel)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		// For each row in the source matrix
		for (size_t row = 0; row < rows; row++)
		{
			// For each col in the source matrix
			for (size_t col = 0; col < cols; col++)
			{
				// Skip the margin of source with the zeroing of result
				if ((row == 0) || (row == rows - 1) || (col == 0) || (col == cols - 1))
				{
					dst.set(row, col, 0);
					continue;
				}

				// Apply kernel by the convolution
				short convolvedResult =
					(short)src.get(row - 1, col - 1) * kernel[0] +
					(short)src.get(row - 1, col    ) * kernel[1] +
					(short)src.get(row - 1, col + 1) * kernel[2] +
					(short)src.get(row    , col - 1) * kernel[3] +
					(short)src.get(row    , col    ) * kernel[4] +
					(short)src.get(row    , col + 1) * kernel[5] +
					(short)src.get(row + 1, col - 1) * kernel[6] +
					(short)src.get(row + 1, col    ) * kernel[7] +
					(short)src.get(row + 1, col + 1) * kernel[8];

				// Convolution result must be trimmed before setting
				byte trimmedResult = (byte)std::min(std::abs(convolvedResult), 255);
				dst.set(row, col, trimmedResult);
			}
		}
	}

	float calculateResponse(ByteMatrix& matrix, size_t x, size_t y, size_t halfKernelSize)
	{
		// Accumulate bilinear derivatives for the further calculation of derivatives matrix elements
		float dxxSum = 0.0F;
		float dxySum = 0.0F;
		float dyySum = 0.0F;

		// For each row in the kernel around central pixel
		for (size_t kernelY = y - halfKernelSize; kernelY < y + halfKernelSize; kernelY++)
		{
			// For each col in the kernel around central pixel
			for (size_t kernelX = x - halfKernelSize; kernelX < x + halfKernelSize; kernelX++)
			{
				// Get pixel values
				byte valCentral = matrix.get(kernelY, kernelX);
				byte valNextRow = matrix.get(kernelY + 1, kernelX);
				byte valNextCol = matrix.get(kernelY, kernelX + 1);

				// Calculate discrete partial derivatives
				float dx = (kernelX == x + halfKernelSize) ? 0.0F : fabsf((float)valNextCol - valCentral);
				float dy = (kernelY == y + halfKernelSize) ? 0.0F : fabsf((float)valNextRow - valCentral);

				// Accumulate bilinear derivatives
				dxxSum += dx * dx;
				dxySum += dx * dy;
				dyySum += dy * dy;
			}
		}

		// Elements of derivatives matrix: averaging performed according to number of accumulated derivatives
		float dxxAvg = dxxSum / (CORNER_DETECTION_KERNEL_SIZE - 1) / CORNER_DETECTION_KERNEL_SIZE;
		float dxyAvg = dxySum / (CORNER_DETECTION_KERNEL_SIZE - 1) / (CORNER_DETECTION_KERNEL_SIZE - 1);
		float dyyAvg = dyySum / (CORNER_DETECTION_KERNEL_SIZE - 1) / CORNER_DETECTION_KERNEL_SIZE;

		// Value of response in Harris corner detector indicates quality of eigenvalues of derivatives matrix
		float determinant = dxxAvg * dyyAvg - dxyAvg * dxyAvg;
		float trace = dxxAvg + dyyAvg;
		float response = fabsf(determinant - m_sensitivity * trace * trace);

		return response;
	}

	void storeCorners(const std::vector<ScoredCorner>& scoredCorners, const std::string& filenameLayer)
	{
		std::ofstream fileLayer(filenameLayer);
		fileLayer << "Row,Col,Score,Gray level" << std::endl;
		for (const ScoredCorner& scoredCorner : scoredCorners)
		{
			size_t row = mm2pixels(scoredCorner.y);
			size_t col = mm2pixels(scoredCorner.x);
			fileLayer << row << "," << col << "," << std::setprecision(6) << scoredCorner.score << "," <<
				(int)scoredCorner.grayLevel << std::endl;
		}
		fileLayer.close();
	}
};
