#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

typedef unsigned char byte;

struct RegressionResult
{
	float slope;
	float offset;
};

const byte BLACK = 0;
const byte DELIM = 128;
const byte WHITE = 255;

// Size of the kernel to find corners
const size_t CORNER_DETECTION_KERNEL_SIZE = 3;

// Kernels to process image - used also to skip unwanted pixels on seams
const size_t FINE_SMOOTHING_KERNEL_SIZE = 5;
const size_t DEEP_SMOOTHING_KERNEL_SIZE = 51;

const size_t MIN_FOUND_CAPILLARIES = 3;
const size_t DESCRIBED_CAPILLARIES = 10;
const size_t MIN_PIXELS_IN_CAPILLARY = 20;

const std::string Z_POS_FILENAME = "TF_vec_col.csv";

size_t mm2pixels(float mm);
float pixels2mm(size_t pixels);
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
