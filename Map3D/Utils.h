#pragma once

#include <string>
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
