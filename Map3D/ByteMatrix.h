#pragma once

#pragma warning(push)
#pragma warning(disable: 5054)
#include <opencv2/opencv.hpp>
#pragma warning(pop)

#include "Utils.h"

class ByteMatrix
{
public:
	ByteMatrix()
	{
		m_rows = 0;
		m_cols = 0;
	}

	ByteMatrix(const size_t rows, const size_t cols)
	{
		m_rows = rows;
		m_cols = cols;
		if ((m_rows <= 0) || (m_cols <= 0))
		{
			m_buffer = nullptr;
			return;
		}
		//TEMPORARILY
		//checkCuda(cudaMallocManaged(&m_buffer, m_rows * m_cols));
		m_buffer = new byte[m_rows * m_cols];
	}

	~ByteMatrix()
	{
		//TEMPORARILY
		//delete[] m_buffer;
	}

	size_t rows()
	{
		return m_rows;
	}

	size_t cols()
	{
		return m_cols;
	}

	byte* getBuffer()
	{
		return m_buffer;
	}

	cv::Mat asCvMatU8()
	{
		return cv::Mat((int)m_rows, (int)m_cols, CV_8U, m_buffer);
	}

	byte get(size_t row, size_t col)
	{
		return m_buffer[m_cols * row + col];
	}

	void set(size_t row, size_t col, byte val)
	{
		m_buffer[m_cols * row + col] = val;
	}

protected:
	size_t m_rows;
	size_t m_cols;
	byte* m_buffer;
};
