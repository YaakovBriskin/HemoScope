#pragma once

#include "Map.h"

class CornerDetector
{
public:
	CornerDetector();
	~CornerDetector();

	void setLayerPosition(float z);

	// Apply Gx and Gy Sobel kernels and find average gradient values over predefined threshold
	std::vector<ScoredCorner> getCornersSobel(Map& map, ByteMatrix& matrix,
		const std::string& capillariesFolderName, size_t layerIndex);

private:
	float m_z;

	// Limit number of rows for the search of corners
	size_t m_croppedRows;

	// Detect corners on the gradient matrix after applocation of Sobel filter
	byte m_gradientThreshold;

	// Minimal allowed distance between detected corners
	size_t m_minDistancePixels;

	// Device memory buffers
	byte* md_srcBuffer;
	byte* md_dstBufferSobelGx;
	byte* md_dstBufferSobelGy;
	byte* md_dstBufferSobel;

private:
	void allocateDeviceBuffers(size_t rows, size_t cols);
	void freeDeviceBuffers();
	void writeCorners(const std::vector<ScoredCorner>& scoredCorners, const std::string& filenameLayer);
};
