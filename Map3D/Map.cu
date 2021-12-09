#include <regex>
#include <iterator>

#include "Utils.h"
#include "UtilsCUDA.h"
#include "Map.h"

/*
	Kernel Device (GPU) variables and functions
	===========================================
*/

/*
	Public Host (CPU) functions to call kernel Device (GPU) functions
	=================================================================
*/

Map::Map()
{
	m_startXmm = 0.0F;
	m_startYmm = 0.0F;
	m_stepXmm = 0.0F;
	m_stepYmm = 0.0F;
	m_rows = 0;
	m_cols = 0;
	m_markerSize = 21;
}

void Map::buildMap(const std::string& folderName)
{
#ifdef _DEBUG
	std::string config = "DEBUG";
#else
	std::string config = "RELEASE";
#endif
	std::cout << "Build 3D map in " << config << " configuration" << std::endl << std::endl;
	std::string absFolderName = getAbsFolderName(folderName);
	std::cout << "Input data folder:" << std::endl << absFolderName << std::endl << std::endl;

	// Vector of X-Y-Z coordinates
	std::vector<std::vector<std::string>> scanPositions = readScanPositions(folderName);

	// Images in the order of above scan positions
	std::cout << "Start loading of " << scanPositions[0].size() << " images" << std::endl;
	m_timer.start();
	std::vector<cv::Mat> images = readImages(folderName);
	m_timer.end();
	std::cout << "Images are loaded in " <<
		m_timer.getDurationMilliseconds() << " ms" << std::endl << std::endl;

	// Allocate byte matrices on each layer
	initLayers(images[0]);

	// Build stitched images on each layer
	std::cout << "Start stitching of " << scanPositions[0].size() << " images" << std::endl;
	m_timer.start();
	stitchImages(scanPositions, images);
	m_timer.end();
	std::cout << "Images are stitched in " <<
		m_timer.getDurationMilliseconds() << " ms" << std::endl << std::endl;
}

void Map::printValueAtTruncatedPos(float x, float y, float z)
{
	std::cout << "Get value on some position truncated to scan grid" << std::endl;

	size_t layersNum = m_layers.size();

	// Validate z
	if (z < m_layers[0].z)
	{
		std::cout << "Position z is less than lower bound" << std::endl;
	}
	if (z > m_layers[layersNum - 1].z)
	{
		std::cout << "Position z is more than upper bound" << std::endl;
	}

	// Find index of the layer for which given z is between iterated and next layer
	size_t layerIndex = 0;
	while (layerIndex < layersNum - 2)
	{
		if ((m_layers[layerIndex].z <= z) && (m_layers[layerIndex + 1].z > z))
		{
			break;
		}
		layerIndex++;
	}

	// Find col that matches truncated x
	float minX = m_indexedPositionsX.begin()->first;
	if (x < minX)
	{
		std::cout << "Position x is less than lower bound" << std::endl;
	}
	size_t col = (size_t)floor((x - minX) / m_stepXmm);
	if (col > m_cols - 1)
	{
		std::cout << "Position x is more than upper bound" << std::endl;
	}

	// Find row that matches truncated y
	float minY = m_indexedPositionsY.begin()->first;
	if (y < minY)
	{
		std::cout << "Position y is less than lower bound" << std::endl;
	}
	size_t row = (size_t)floor((y - minY) / m_stepYmm);
	if (row > m_rows - 1)
	{
		std::cout << "Position y is more than upper bound" << std::endl;
	}

	byte val = m_layers[layerIndex].matrix.get(row, col);
	std::cout << "x = " << x << std::endl << "y = " << y << std::endl << "z = " << z << std::endl <<
		"value = " << (int)val << std::endl << std::endl;
}

void Map::saveStiched(std::vector<LayerInfo>& layersWithCapillaries, const std::string& outputFolderName)
{
	createFoldersIfNeed(outputFolderName, "Stitched");
	size_t layersNum = m_layers.size();
	std::cout << "Start saving of stitched images on " << layersNum << " layers" << std::endl;
	m_timer.start();

	if (layersWithCapillaries.empty())
	{
		// Called before capillaries detection: save all layers without detectied capillaries
		for (size_t layerIndex = 0; layerIndex < layersNum; layerIndex++)
		{
			ByteMatrix layerMatrix = m_layers[layerIndex].matrix;
			std::string layerFilename = outputFolderName + "/Stitched/Layer" +
				std::to_string(layerIndex + 1) + ".bmp";
			bool result = cv::imwrite(layerFilename, layerMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + layerFilename).c_str());
			}
		}
	}
	else
	{
		// Called after capillaries detection: save actual layers with detectied capillaries
		for (const LayerInfo& layerInfo : layersWithCapillaries)
		{
			ByteMatrix layerMatrix = m_layers[layerInfo.layerIndex].matrix;
			std::vector<ScoredCorner> scoredCorners = layerInfo.capillaryApexes;
			markCorners(layerMatrix, scoredCorners);
			std::string layerFilename = outputFolderName + "/Stitched/LayerDetected" +
				std::to_string(layerInfo.layerIndex + 1) + ".bmp";
			bool result = cv::imwrite(layerFilename, layerMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + layerFilename).c_str());
			}
		}
	}

	m_timer.end();
	std::cout << "Stitched images are saved in " << m_timer.getDurationMilliseconds() << " ms" << std::endl << std::endl;
}

bool Map::isOnSeam(size_t posPixels, bool isRow)
{
	return isRow ?
		std::find(m_seamRows.begin(), m_seamRows.end(), posPixels) != m_seamRows.end() :
		std::find(m_seamCols.begin(), m_seamCols.end(), posPixels) != m_seamCols.end();
}

std::vector<Layer> Map::getLayers()
{
	return m_layers;
}

float Map::getStartXmm()
{
	return m_startXmm;
}

float Map::getStartYmm()
{
	return m_startYmm;
}

/*
	Private Host (CPU) functions to call kernel Device (GPU) functions
	==================================================================
*/

std::vector<std::vector<std::string>> Map::readScanPositions(const std::string& folderName)
{
	// Open file with scan positions
	std::string scanPosPathFilename = folderName + "/" + SCAN_POS_FILENAME;
	std::ifstream scanPosFile(scanPosPathFilename);

	// Parse scan positions into intermediate array of X-Y-Z coordinates
	std::string line;
	const std::regex comma(",");
	size_t scansNum = 0;
	std::vector<std::vector<std::string>> scanPositions;
	while (!scanPosFile.eof())
	{
		getline(scanPosFile, line);
		if (line.empty())
		{
			break;
		}
		std::sregex_token_iterator tokenIterator(line.begin(), line.end(), comma, -1);
		std::vector<std::string> coords{ tokenIterator, std::sregex_token_iterator() };
		scanPositions.push_back(coords);
		if (scansNum == 0)
		{
			scansNum = coords.size();
		}
		else
		{
			if (scansNum != coords.size())
			{
				throw std::exception("Mismatch number of coordinates");
			}
		}
	}

	// Get unique scan positions in all X-Y-Z coordinates with sequential indexes
	m_indexedPositionsX = getUniqueIndexedPositions(scanPositions[0]);
	m_indexedPositionsY = getUniqueIndexedPositions(scanPositions[1]);
	m_indexedPositionsZ = getUniqueIndexedPositions(scanPositions[2]);

	// Calculate the step in mm by X as difference of sequential X positions
	std::map<float, size_t>::iterator itrPositionsX = m_indexedPositionsX.begin();
	float initX = itrPositionsX->first;
	itrPositionsX++;
	float nextX = itrPositionsX->first;
	m_stepXmm = nextX - initX;

	// Calculate the step in mm by Y as difference of sequential Y positions
	std::map<float, size_t>::iterator itrPositionsY = m_indexedPositionsY.begin();
	float initY = itrPositionsY->first;
	itrPositionsY++;
	float nextY = itrPositionsY->first;
	m_stepYmm = nextY - initY;

	return scanPositions;
}

std::map<float, size_t> Map::getUniqueIndexedPositions(const std::vector<std::string>& coords)
{
	std::vector<float> uniquePositions;
	for (std::string coord : coords)
	{
		float val = (float)atof(coord.c_str());
		if (std::find(uniquePositions.begin(), uniquePositions.end(), val) == uniquePositions.end())
		{
			uniquePositions.push_back(val);
		}
	}
	std::sort(uniquePositions.begin(), uniquePositions.end());

	std::map<float, size_t> indexedPositions;
	size_t index = 0;
	for (float uniquePosition : uniquePositions)
	{
		indexedPositions[uniquePosition] = index++;
	}

	return indexedPositions;
}

std::vector<cv::Mat> Map::readImages(const std::string& folderName)
{
	size_t filesNum = getFilesNum(folderName);

	const size_t filenameSize = 32;
	char filename[filenameSize];
	std::vector<cv::Mat> images;
	for (size_t fileIndex = 0; fileIndex < filesNum; fileIndex++)
	{
		sprintf_s(filename, filenameSize, "Bright%4d.tif", (int)fileIndex);
		std::string pathFilename = folderName + "/" + filename;
		cv::Mat image = cv::imread(pathFilename, cv::IMREAD_GRAYSCALE);

		images.push_back(image);
		if ((fileIndex > 0) && (fileIndex % 20 == 0))
		{
			std::cout << "Loaded " << std::setw(3) << fileIndex << " images" << std::endl;
		}
	}

	return images;
}

void Map::initLayers(const cv::Mat& firstImage)
{
	m_cols = (size_t)((mm2pixels(m_stepXmm) + BIAS_X_PIXELS) * (m_indexedPositionsX.size() - 1) +
		FRAME_REL_W * firstImage.cols);
	m_rows = (size_t)(mm2pixels(m_stepYmm) * (m_indexedPositionsY.size() - 1) +
		FRAME_REL_H * firstImage.rows - 2 * (m_indexedPositionsX.size() - 1) * BIAS_Y_PIXELS);
	for (std::pair<float, size_t> indexedPositionZ : m_indexedPositionsZ)
	{
		float z = indexedPositionZ.first;
		Layer layer(z, m_rows, m_cols);
		m_layers.push_back(layer);
	}
}

void Map::stitchImages(const std::vector<std::vector<std::string>>& scanPositions, const std::vector<cv::Mat>& images)
{
	// Convert steps from mm to pixels and add preliminarly known biases if need
	const size_t stepPixelsX = mm2pixels(m_stepXmm) + BIAS_X_PIXELS;
	const size_t stepPixelsY = mm2pixels(m_stepYmm);

	// Positions by coordinates
	const std::vector<std::string>& positionsX = scanPositions[0];
	const std::vector<std::string>& positionsY = scanPositions[1];
	const std::vector<std::string>& positionsZ = scanPositions[2];

	// Store start position with initial margins for further calculation of capillaries positions
	m_startXmm = (float)atof(positionsX[0].c_str()) + pixels2mm((size_t)(MARGIN_REL_X * images[0].cols));
	m_startYmm = (float)atof(positionsY[0].c_str()) + pixels2mm((size_t)(MARGIN_REL_Y * images[0].rows));

	// For all positions and corresponding images
	for (size_t imageIndex = 0; imageIndex < images.size(); imageIndex++)
	{
		// Position of iterated source image
		float x = (float)atof(positionsX[imageIndex].c_str());
		float y = (float)atof(positionsY[imageIndex].c_str());
		float z = (float)atof(positionsZ[imageIndex].c_str());

		// Index is in flipped direction by X and in the same direction by Y
		size_t indexX = m_indexedPositionsX.size() - 1 - m_indexedPositionsX[x];
		size_t indexY = m_indexedPositionsY[y];

		// Calculate offsets in the desination image and store to skip unwanted corners on seams
		size_t dstOffsetX = stepPixelsX * indexX;
		size_t dstOffsetY = stepPixelsY * indexY + BIAS_Y_PIXELS * indexX;

		// Store all cols in kernel neighborhood to avoid false-positive corners around seams
		if ((dstOffsetX > 0) && !isOnSeam(dstOffsetX, false))
		{
			for (size_t col = dstOffsetX - DEEP_SMOOTHING_KERNEL_SIZE; col <= dstOffsetX + DEEP_SMOOTHING_KERNEL_SIZE; col++)
			{
				m_seamCols.push_back(col);
			}
		}

		// Store all rows in kernel neighborhood to avoid false-positive corners around seams
		if ((dstOffsetY > 0) && !isOnSeam(dstOffsetY, true))
		{
			for (size_t row = dstOffsetY - DEEP_SMOOTHING_KERNEL_SIZE; row <= dstOffsetY + DEEP_SMOOTHING_KERNEL_SIZE; row++)
			{
				m_seamRows.push_back(row);
			}
		}

		// Frame width is non-onerlapped vertical area for all frames before last or whole last frame
		size_t frameW = (indexX < m_indexedPositionsX.size() - 1) ?
			stepPixelsX :
			(size_t)(FRAME_REL_W * images[imageIndex].cols);

		// Frame height is non-onerlapped horizontal area for all frames before last or whole last frame
		size_t frameH = (indexY < m_indexedPositionsY.size() - 1) ?
			stepPixelsY :
			(size_t)(FRAME_REL_H * images[imageIndex].rows);

		// Select destination layer according to z
		size_t layerIndex = m_indexedPositionsZ[z];
		ByteMatrix& dstMatrix = m_layers[layerIndex].matrix;

		// Copy pixels from source to destination matrix
		stitchSingleImage(dstMatrix, images[imageIndex], dstOffsetX, dstOffsetY, frameW, frameH);
	}
}

void Map::stitchSingleImage(ByteMatrix& dstMatrix, const cv::Mat& srcImage, const size_t dstOffsetX, const size_t dstOffsetY,
	const size_t frameW, const size_t frameH)
{
	// Convert offsets from mm to pixel
	size_t srcOffsetRow = (size_t)(MARGIN_REL_Y * srcImage.rows);
	size_t srcOffsetCol = (size_t)(MARGIN_REL_X * srcImage.cols);

	// For all rows in the source frame
	for (size_t srcRow = 0; srcRow < frameH; srcRow++)
	{
		// Crop destination image
		int dstOffsetRow = (int)(dstOffsetY + srcRow) - (int)((m_indexedPositionsX.size() - 1) * BIAS_Y_PIXELS);
		if ((dstOffsetRow < 0) || (dstOffsetRow > m_rows - 1))
		{
			continue;
		}

		// Copy to destination image
		for (size_t srcCol = 0; srcCol < frameW; srcCol++)
		{
			byte val = srcImage.at<byte>((int)(srcOffsetRow + srcRow), (int)(srcOffsetCol + srcCol));
			dstMatrix.set((size_t)dstOffsetRow, dstOffsetX + srcCol, val);
		}
	}
}

void Map::copyScanPosFile(const std::string& scanPosFolderName, const std::string& outputFolderName)
{
	std::string scanPosSrcPathFilename = scanPosFolderName + "/" + SCAN_POS_FILENAME;
	std::string scanPosDstPathFilename = outputFolderName + "/Stitched/" + SCAN_POS_FILENAME;
	copyFile(scanPosSrcPathFilename, scanPosDstPathFilename);
}

// For debugging purpose only
void Map::markCorners(ByteMatrix& matrix, std::vector<ScoredCorner>& scoredCorners)
{
	// Number of pixels around the central pixel for valid marker odd sizes
	size_t halfMarkerSize = m_markerSize / 2;

	for (const ScoredCorner& scoredCorner : scoredCorners)
	{
		// Convert position of the corner from mm to pixels
		size_t cornerRow = mm2pixels(scoredCorner.y);
		size_t cornerCol = mm2pixels(scoredCorner.x);

		// Limit marker boundaries to not excess the matrix
		size_t markerRowMin = std::max<size_t>(cornerRow - halfMarkerSize, 0);
		size_t markerRowMax = std::min<size_t>(cornerRow + halfMarkerSize, matrix.rows() - 1);
		size_t markerColMin = std::max<size_t>(cornerCol - halfMarkerSize, 0);
		size_t markerColMax = std::min<size_t>(cornerCol + halfMarkerSize, matrix.cols() - 1);

		// Paint over the marker
		for (size_t row = markerRowMin; row <= markerRowMax; row++)
		{
			for (size_t col = markerColMin; col <= markerColMax; col++)
			{
				matrix.set(row, col, WHITE);
			}
		}
	}
}
