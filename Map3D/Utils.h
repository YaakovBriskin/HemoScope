#pragma once

#include <string>
#include <vector>
#include <memory>
#include <numbers>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "Config.h"

typedef unsigned char byte;

struct RegressionResult
{
	float slope;
	float offset;
};

// Used colors
const byte BLACK = 0;
const byte DELIM = 128;
const byte WHITE = 255;
const byte LIGHT_GRAY = 0xCD;

// Size of Sobel kernels to find corners
const size_t CORNER_DETECTION_KERNEL_SIZE = 3;

// General data from configuration
static size_t pixelsInMm;

// Locally used data from configuration
static byte grayLevelOriginalMin;
static byte grayLevelOriginalMax;
static byte grayLevelProcessedMin;
static byte grayLevelProcessedMax;

void initGeneralData(Config& config);
size_t mm2pixels(float mm);
float pixels2mm(size_t pixels);
size_t rad2deg(float angleRadians);
float deg2rad(size_t angleDegrees);
float makeCentrosymmetric(float angleRadians);
bool isValidGrayLevelOriginal(byte val);
bool isValidGrayLevelProcessed(byte val);
std::string toString(const float val, const int n = 3);
std::string getAbsFolderName(const std::string& folderName);
void createFoldersIfNeed(const std::string& folderName, const std::string& subFolderName);
void copyFile(const std::string& srcFilename, const std::string& dstFilename);
size_t getFilesNum(const std::string& folderName);
RegressionResult calculateRegression(std::vector<float>& imageMarkers, std::vector<float>& positionsZ);
void saveResults(std::vector<float>& positionsZ, std::vector<float>& modeIndices,
	RegressionResult& result, const std::string& outputFolderName, bool isHalf = false);
