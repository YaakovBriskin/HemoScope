#pragma once

#include <numbers>
#include "ByteMatrix.h"

const byte BLACK = 0;
const size_t FRAME_WIDTH = 40;
const size_t FRAME_HEIGHT = 100;
const float SCORE_THRESHOLD = 0.9F;

class PixelPos
{
public:
	size_t pixelRow;
	size_t pixelCol;

public:
	PixelPos()
	{
		pixelRow = 0;
		pixelCol = 0;
	}

	PixelPos(size_t row, size_t col)
	{
		pixelRow = row;
		pixelCol = col;
	}
};

class MaxRectangle
{
public:
	MaxRectangle(ByteMatrix& byteMatrix, PixelPos start, size_t rows, size_t cols,
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

	std::vector<PixelPos> findRectangle(const std::string& layerFolderName, size_t capillaryIndex)
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
		m_foundAngleRadians = -(float)angleDegrees / 180.0F * (float)std::numbers::pi;

		rotatedRectangle = getRotatedRectangle();
		return rotatedRectangle;
	}

	float getAngle()
	{
		return m_foundAngleRadians;
	}

	float getScore()
	{
		return 100.0F * (m_score - SCORE_THRESHOLD);
	}

private:
	ByteMatrix m_originalCapillary;
	ByteMatrix m_rotatedCapillary;
	ByteMatrix m_dilatedCapillary;
	PixelPos m_centerInImage;

	size_t m_limitUp;
	size_t m_limitDn;
	size_t m_limitLf;
	size_t m_limitRt;

	size_t m_rowFrameInRotated;
	size_t m_colFrameInRotated;
	float m_foundAngleRadians;
	float m_score;

private:
	void rotateCapillary(size_t angleDegrees)
	{
		// Convert given angle to radians
		float angle = (float)angleDegrees / 180.0F * (float)std::numbers::pi;

		// Reset previous rotation
		m_rotatedCapillary.clean();

		size_t rowsSrc = m_originalCapillary.rows();
		size_t colsSrc = m_originalCapillary.cols();

		size_t centerRowSrc = rowsSrc / 2;
		size_t centerColSrc = colsSrc / 2;

		size_t centerRowDst = m_rotatedCapillary.rows() / 2;
		size_t centerColDst = m_rotatedCapillary.cols() / 2;

		for (size_t rowSrc = 0; rowSrc < rowsSrc; rowSrc++)
		{
			for (size_t colSrc = 0; colSrc < colsSrc; colSrc++)
			{
				byte pixel = m_originalCapillary.get(rowSrc, colSrc);
				if (pixel != WHITE)
				{
					continue;
				}
				float deltaY = (float)centerRowSrc - (float)rowSrc;
				float deltaX = (float)centerColSrc - (float)colSrc;
				float radius = std::sqrtf(deltaX * deltaX + deltaY * deltaY);
				float angleSrc = std::atan2f(deltaY, deltaX);
				float angleDst = angleSrc + angle + (float)std::numbers::pi; // axis Y is counter-directional to rows numeration
				size_t rowDst = (size_t)((int)centerRowDst + (int)std::round(radius * std::sinf(angleDst)));
				size_t colDst = (size_t)((int)centerColDst + (int)std::round(radius * std::cosf(angleDst)));
				m_rotatedCapillary.set(rowDst, colDst, pixel);
			}
		}
	}

	void findCapillaryLimits()
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

	void dilateRotatedCapillary()
	{
		const size_t dilationKernemSize = 3;
		size_t halfKernelSize = dilationKernemSize / 2;
		size_t threshold = dilationKernemSize * dilationKernemSize / 2 - 1;

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

	bool findInscribedRectangle()
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

	void markFrameInDilatedCapillary(bool foundInscribedRectangle)
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

	void writeWidthMap(const std::string& layerFolderName, size_t capillaryIndex)
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

	std::vector<PixelPos> getRotatedRectangle()
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
			float angleDst = angleSrc + m_foundAngleRadians + (float)std::numbers::pi; // axis Y is counter-directional to rows numeration
			size_t rowInImage = (size_t)((int)m_centerInImage.pixelRow + (int)std::round(radius * std::sinf(angleDst)));
			size_t colInImage = (size_t)((int)m_centerInImage.pixelCol + (int)std::round(radius * std::cosf(angleDst)));
			rotatedRectangle.push_back(PixelPos(rowInImage, colInImage));
		}

		return rotatedRectangle;
	}
};
