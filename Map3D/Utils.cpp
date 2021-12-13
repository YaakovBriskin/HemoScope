#include "Utils.h"

void initGeneralData(Config& config)
{
	pixelsInMm = (size_t)config.getIntValue(keyPixelsInMm);
	grayLevelOriginalMin = (byte)config.getIntValue(keyGrayLevelOriginalMin);
	grayLevelOriginalMax = (byte)config.getIntValue(keyGrayLevelOriginalMax);
	grayLevelProcessedMin = (byte)config.getIntValue(keyGrayLevelProcessedMin);
	grayLevelProcessedMax = (byte)config.getIntValue(keyGrayLevelProcessedMax);
}

size_t mm2pixels(float mm)
{
	return (size_t)std::roundf(pixelsInMm * mm);
}

float pixels2mm(size_t pixels)
{
	return (float)pixels / pixelsInMm;
}

size_t rad2deg(float angleRadians)
{
	return (size_t)std::roundf(angleRadians / (float)std::numbers::pi * 180.0F);
}

float deg2rad(size_t angleDegrees)
{
	return (float)angleDegrees / 180.0F * (float)std::numbers::pi;
}

float makeCentrosymmetric(float angleRadians)
{
	return angleRadians + (float)std::numbers::pi;
}

// Examine gray level to find possible capillary corner - performed on raw image
bool isValidGrayLevelOriginal(byte val)
{
	return (grayLevelOriginalMin <= val) && (val <= grayLevelOriginalMax);
}

// Examine gray level to find limit of capillary - performed on processed image
bool isValidGrayLevelProcessed(byte val)
{
	return (grayLevelProcessedMin <= val) && (val <= grayLevelProcessedMax);
}

std::string toString(const float val, const int n)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << val;
	return out.str();
}

std::string getAbsFolderName(const std::string& folderName)
{
	std::filesystem::path folder = std::filesystem::absolute(std::filesystem::path(folderName));
	std::string absFolderName = folder.generic_string();
	std::replace(absFolderName.begin(), absFolderName.end(), '/', '\\');
	return absFolderName;
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

void copyFile(const std::string& srcFilename, const std::string& dstFilename)
{
	std::filesystem::path srcFile(srcFilename);
	std::filesystem::path dstFile(dstFilename);
	bool result = std::filesystem::copy_file(srcFile, dstFile,
		std::filesystem::copy_options::overwrite_existing);
	if (!result)
	{
		throw std::exception(("Cannot copy file: " + srcFilename).c_str());
	}
}

size_t getFilesNum(const std::string& folderName)
{
	// Define parameters of the folder
	std::filesystem::path imagesPath(folderName);
	std::filesystem::directory_iterator folderIterator(imagesPath);
	std::filesystem::directory_iterator emptyIterator;

	// Get number of files in the folder and reduce by the CSV file of scan positions
	size_t filesNum = std::distance(folderIterator, emptyIterator);
	filesNum--;
	return filesNum;
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

	RegressionResult result{};
	result.slope = (n * sumXY - sumX1 * sumY1) / (n * sumX2 - sumX1 * sumX1);
	result.offset = (sumX2 * sumY1 - sumXY * sumX1) / (n * sumX2 - sumX1 * sumX1);
	return result;
}

void saveResults(std::vector<float>& positionsZ, std::vector<float>& modeIndices,
	RegressionResult& result, const std::string& outputFolderName, bool isHalf)
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
