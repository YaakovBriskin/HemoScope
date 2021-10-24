#pragma once

#include "ByteMatrix.h"

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
	MaxRectangle(const ByteMatrix& byteMatrix)
	{
		m_originalMatrix = byteMatrix;
		//m_rotatedMatrix
	}

private:
	ByteMatrix m_originalMatrix;
	ByteMatrix m_rotatedMatrix;
};