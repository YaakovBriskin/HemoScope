#pragma once

#pragma warning(push)
#pragma warning(disable: 5054)
#include <opencv2/opencv.hpp>
#pragma warning(pop)

#include "Utils.h"

class ByteMatrix
{
public:
	ByteMatrix();
	ByteMatrix(const size_t rows, const size_t cols);
	size_t rows();
	size_t cols();
	byte* getBuffer();
	cv::Mat asCvMatU8();
	byte get(size_t row, size_t col);
	void set(size_t row, size_t col, byte val);
	void clean();

protected:
	size_t m_rows;
	size_t m_cols;
	std::shared_ptr<byte[]> m_buffer;
};
