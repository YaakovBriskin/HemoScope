#pragma once

#include <string>
#include <vector>
#include <map>

#pragma warning(push)
#pragma warning(disable: 5054)
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#pragma warning(pop)

#include "Timer.h"
#include "Point3D.h"
#include "ByteMatrix.h"

class Layer
{
public:
	float z = 0.0F;
	ByteMatrix matrix = ByteMatrix();

public:
	Layer(const float layerZ, const size_t rows, const size_t cols)
	{
		z = layerZ;
		matrix = ByteMatrix(rows, cols);
	}
};

class ScoredCorner : public Point3D
{
public:
	float score;
	byte grayLevel;

public:
	ScoredCorner(float x, float y, float z, float assignedScore, byte grayLevelVal) :
		Point3D(x, y, z)
	{
		score = assignedScore;
		grayLevel = grayLevelVal;
	}
};

class CapillaryInfo
{
public:
	size_t index;
	Point3D posApex;
	size_t limitUp;
	size_t limitDn;
	size_t limitLf;
	size_t limitRt;
	size_t pixelsCapillary;
	size_t energyCapillary;
	size_t pixelsSurroundings;
	size_t energySurroundings;
	float angle;
	float score;

public:
	CapillaryInfo()
	{
		index = 0;
		limitUp = 0;
		limitDn = 0;
		limitLf = 0;
		limitRt = 0;
		pixelsCapillary = 0;
		energyCapillary = 0;
		pixelsSurroundings = 0;
		energySurroundings = 0;
		angle = 0.0F;
		score = 0.0F;
	}

	void setPos(const ScoredCorner& scoredCorner)
	{
		posApex.x = scoredCorner.x;
		posApex.y = scoredCorner.y;
		posApex.z = scoredCorner.z;
	}
};

class LayerInfo
{
public:
	size_t layerIndex;
	float z;

	// Detected corners with the score of each
	std::vector<ScoredCorner> capillaryApexes;

	// Description of capillaries in the layer
	std::vector<CapillaryInfo> capillariesInfo;

	// Score of the best capillary in the layer
	float maxScore;

	// Scores of all capillaries in the layer
	float sumScore;
};

class Map
{
public:
	Map();
	~Map() = default;

	void buildMap(const std::string& folderName, Config& config);
	void printValueAtTruncatedPos(float x, float y, float z);
	void saveStiched(std::vector<LayerInfo>& layersWithCapillaries, const std::string& outputFolderName);
	bool isOnSeam(size_t posPixels, bool isRow);
	std::vector<Layer> getLayers();
	float getStartXmm();
	float getStartYmm();

private:
	std::vector<Layer> m_layers;

	std::map<float, size_t> m_indexedPositionsX;
	std::map<float, size_t> m_indexedPositionsY;
	std::map<float, size_t> m_indexedPositionsZ;

	// Parameters from configuration
	std::string m_scanPosFilename;
	size_t m_markerCornerSize;
	size_t m_imageBiasPixelsX;
	size_t m_imageBiasPixelsY;
	float m_imageMarginRelativeX;
	float m_imageMarginRelativeY;
	float m_imageFrameRelativeW;
	float m_imageFrameRelativeH;

	float m_startXmm;
	float m_startYmm;

	float m_stepXmm;
	float m_stepYmm;

	size_t m_rows;
	size_t m_cols;

	std::vector<size_t> m_seamRows;
	std::vector<size_t> m_seamCols;

	Timer m_timer;

private:
	void initConfig(Config& config);
	std::vector<std::vector<std::string>> readScanPositions(const std::string& folderName);
	std::map<float, size_t> getUniqueIndexedPositions(const std::vector<std::string>& coords);
	std::vector<cv::Mat> readImages(const std::string& folderName);
	void initLayers(const cv::Mat& firstImage);
	void stitchImages(const std::vector<std::vector<std::string>>& scanPositions,
		const std::vector<cv::Mat>& images, size_t deepSmoothingKernelSize);
	void stitchSingleImage(ByteMatrix& dstMatrix, const cv::Mat& srcImage,
		const size_t dstOffsetX, const size_t dstOffsetY, const size_t frameW, const size_t frameH);
	void copyScanPosFile(const std::string& scanPosFolderName, const std::string& outputFolderName);

	// For debugging purpose only
	void markCorners(ByteMatrix& matrix, std::vector<ScoredCorner>& scoredCorners);
};
