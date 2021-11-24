#pragma once

#include <string>
#include <fstream>
#include <filesystem>

typedef unsigned char byte;

const size_t PIXELS_IN_MM = 2600;

size_t mm2pixels(float mm)
{
	return (size_t)std::roundf(PIXELS_IN_MM * mm);
}

float pixels2mm(size_t pixels)
{
	return (float)pixels / PIXELS_IN_MM;
}

const byte BLACK = 0;
const byte DELIM = 128;
const byte WHITE = 255;

// Size of the kernel to find corners
const size_t CORNER_DETECTION_KERNEL_SIZE = 3;

// Kernels to process image - used also to skip unwanted pixels on seams
const size_t FINE_SMOOTHING_KERNEL_SIZE = 5;
const size_t DEEP_SMOOTHING_KERNEL_SIZE = 51;

const std::string Z_POS_FILENAME = "TF_vec_col.csv";

struct RegressionResult
{
	float slope;
	float offset;
};

// Examine gray level to find possible capillary corner - performed on raw image
bool isValidGrayLevelOriginal(byte val)
{
	const byte glMin = 30;
	const byte glMax = 90;
	return (glMin <= val) && (val <= glMax);
}

// Examine gray level to find limit of capillary - performed on processed image
bool isValidGrayLevelProcessed(byte val)
{
	const byte glMin = 0;
	const byte glMax = 120;
	return (glMin <= val) && (val <= glMax);
}

const size_t MIN_FOUND_CAPILLARIES = 3;
const size_t DESCRIBED_CAPILLARIES = 10;
const size_t MIN_PIXELS_IN_CAPILLARY = 20;

std::string toString(const float val, const int n = 3)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << val;
	return out.str();
}

void createFoldersIfNeed(const std::string& folderName, const std::string& subFolderName)
{
	// Check existence of folder and create if it does not exist
	bool result = true;
	if (!std::filesystem::exists(std::filesystem::path(folderName)))
	{
		result = std::filesystem::create_directory(std::filesystem::path(folderName));
		if (!result)
		{
			throw std::exception(("Cannot create folder: " + folderName).c_str());
		}
	}

	// Check existence of subfolder and create if it does not exist
	const std::string nestedFolderName = folderName + "/" + subFolderName;
	if (!std::filesystem::exists(std::filesystem::path(nestedFolderName)))
	{
		result = std::filesystem::create_directory(std::filesystem::path(nestedFolderName));
		if (!result)
		{
			throw std::exception(("Cannot create folder: " + nestedFolderName).c_str());
		}
	}
}

RegressionResult calculateRegression(std::vector<float>& imageMarkers, std::vector<float>& positionsZ)
{
	size_t n = imageMarkers.size();
	if (positionsZ.size() != n)
	{
		throw std::exception("Data sizes mismatch");
	}

	float sumX1 = 0.0F;
	float sumX2 = 0.0F;
	float sumY1 = 0.0F;
	float sumXY = 0.0F;
	for (size_t i = 0; i < n; i++)
	{
		sumX1 += imageMarkers[i];
		sumX2 += imageMarkers[i] * imageMarkers[i];
		sumY1 += positionsZ[i];
		sumXY += imageMarkers[i] * positionsZ[i];
	}

	RegressionResult result;
	result.slope = (n * sumXY - sumX1 * sumY1) / (n * sumX2 - sumX1 * sumX1);
	result.offset = (sumX2 * sumY1 - sumXY * sumX1) / (n * sumX2 - sumX1 * sumX1);
	return result;
}

void saveResults(std::vector<float>& positionsZ, std::vector<float>& modeIndices,
	RegressionResult& result, const std::string& outputFolderName, bool isHalf = false)
{
	std::string positionsFilename = outputFolderName + "/PositionsZ" +
		+(isHalf ? "Half" : "") + ".csv";
	std::ofstream positionsFile(positionsFilename);
	positionsFile << "Index,Z given,Z calculated" << std::endl;

	for (size_t posIndex = 0; posIndex < positionsZ.size(); posIndex++)
	{
		float positionCalc = result.slope * modeIndices[posIndex] + result.offset;
		positionsFile <<
			posIndex << "," <<
			positionsZ[posIndex] << "," <<
			positionCalc << std::endl;
	}

	positionsFile.close();
}
