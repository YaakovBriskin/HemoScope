#pragma once

#include "Map.h"
#include "MaxRectangle.h"

class CapillaryProcessor
{
public:
	CapillaryProcessor();

	void describeCapillaries(Map& map, LayerInfo& layerInfo, const std::string& outputFolderName);

private:
	ByteMatrix m_originalMatrix;
	ByteMatrix m_processedMatrix;
	size_t m_layerIndex;
	Timer m_timer;

private:
	void performGaussianBlur(ByteMatrix& src, ByteMatrix& dst);
	void performUniformSmoothing(ByteMatrix& src, ByteMatrix& dst);
	void performExcessFiltering(ByteMatrix& src, ByteMatrix& dst);

	/*
		Perform traversal of connected pixels in the area from given root: row and col.
		Pixels in the area assumed have valid gray level which means predefined interval.
		The area traversal is performed using the BFS (Breadth-First Search) algorithn.
		This algorithm is based on non-recursive iteration in loop with usage of queue.
		Recursive traversal with the DFS (Depth-First Search) leads to stack overflow.
	*/
	void performTraversalBFS(size_t row, size_t col, Map& map, CapillaryInfo& capillaryInfo);

	void processPixel(const PixelPos& pixelPos, CapillaryInfo& capillaryInfo);
	void collectSurroundings(std::vector<CapillaryInfo>& capillariesInfo);
	void updateSurroundingData(CapillaryInfo& capillaryInfo,
		size_t rectUp, size_t rectDn, size_t rectLf, size_t rectRt);
	void trimAndSetLayerScores(LayerInfo& layerInfo, float startXmm, float startYmm,
		std::vector<CapillaryInfo>& capillariesInfo, const std::string& layerFolderName);
	void drawRotatedFrame(const std::vector<PixelPos>& rotatedFrame);
	void drawLine(PixelPos pixelA, PixelPos pixelB);
};
