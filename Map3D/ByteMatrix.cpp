#include "UtilsCUDA.h"
#include "ByteMatrix.h"

ByteMatrix::ByteMatrix()
{
	m_rows = 0;
	m_cols = 0;
	m_buffer = nullptr;
}

ByteMatrix::ByteMatrix(const size_t rows, const size_t cols)
{
	m_rows = rows;
	m_cols = cols;
	if ((m_rows <= 0) || (m_cols <= 0))
	{
		m_buffer = nullptr;
		return;
	}
	m_buffer = new byte[m_rows * m_cols];
}

ByteMatrix::~ByteMatrix()
{
	//delete[] m_buffer;
}

size_t ByteMatrix::rows()
{
	return m_rows;
}

size_t ByteMatrix::cols()
{
	return m_cols;
}

byte* ByteMatrix::getBuffer()
{
	return m_buffer;
}

cv::Mat ByteMatrix::asCvMatU8()
{
	return cv::Mat((int)m_rows, (int)m_cols, CV_8U, m_buffer);
}

byte ByteMatrix::get(size_t row, size_t col)
{
	return m_buffer[m_cols * row + col];
}

void ByteMatrix::set(size_t row, size_t col, byte val)
{
	m_buffer[m_cols * row + col] = val;
}

void ByteMatrix::clean()
{
	memset(m_buffer, 0xCD, m_rows * m_cols);
}
