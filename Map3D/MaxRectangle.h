#pragma once

#include <string>
#include <vector>

#include "ByteMatrix.h"

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
		const std::string& layerFolderName, size_t capillaryIndex);

	std::vector<PixelPos> findRectangle(const std::string& layerFolderName, size_t capillaryIndex);
	float getAngle();
	float getScore();

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
	void rotateCapillary(size_t angleDegrees);
	void findCapillaryLimits();
	void dilateRotatedCapillary();
	bool findInscribedRectangle();
	void markFrameInDilatedCapillary(bool foundInscribedRectangle);
	void writeWidthMap(const std::string& layerFolderName, size_t capillaryIndex);
	std::vector<PixelPos> getRotatedRectangle();
};
