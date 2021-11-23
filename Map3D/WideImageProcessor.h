#pragma once


#include <vector>
#include <fstream>

#include "Sequence.h"

#pragma warning(disable: 26812)

enum ImageMarkerType
{
	GRAY_LEVEL_MODE,
	GRAY_LEVEL_VARIANCE
};

class WideImageProcessor
{
public:
	void calculateStatistics(const std::string& imagesFolderName, const std::string& outputFolderName,
		std::vector<float>& positionsZ)
	{
		std::filesystem::path inputFolder = std::filesystem::absolute(std::filesystem::path(imagesFolderName));
		std::string absFolderName = inputFolder.generic_string();
		std::replace(absFolderName.begin(), absFolderName.end(), '/', '\\');
		std::cout << "Input data folder:" << std::endl << absFolderName << std::endl << std::endl;

		const size_t fileNameSize = 32;
		char inputFilename[fileNameSize];
		createFoldersIfNeed(outputFolderName, "Histogram");

		std::string statisticsFilename = outputFolderName + "/Histogram/Statistics.csv";
		std::ofstream statisticsFile(statisticsFilename);

		std::vector<float> imageMarkers;
		for (size_t fileIndex = 0; fileIndex < positionsZ.size(); fileIndex++)
		{
			sprintf_s(inputFilename, fileNameSize, "Bright%4d.tif", (int)fileIndex);
			cv::Mat wideImage = cv::imread(imagesFolderName + "/" + inputFilename, cv::IMREAD_GRAYSCALE);

			std::string histogramFilename = outputFolderName + "/Histogram/Histogram" +
				std::to_string(fileIndex) + ".csv";
			std::ofstream histogramFile(histogramFilename);

			float imageMarker = getImageMarker(wideImage, ImageMarkerType::GRAY_LEVEL_VARIANCE,
				histogramFile, statisticsFile);
			histogramFile.close();
			imageMarkers.push_back(imageMarker);
		}

		statisticsFile.close();

		RegressionResult result = calculateRegression(imageMarkers, positionsZ);
		saveResults(positionsZ, imageMarkers, result, outputFolderName);
	}

	void calculateStatisticsHalf(const std::string& imagesFolderName, const std::string& outputFolderName,
		std::vector<float>& positionsZ)
	{
		std::filesystem::path inputFolder = std::filesystem::absolute(std::filesystem::path(imagesFolderName));
		std::string absFolderName = inputFolder.generic_string();
		std::replace(absFolderName.begin(), absFolderName.end(), '/', '\\');
		std::cout << "Input data folder:" << std::endl << absFolderName << std::endl << std::endl;

		const size_t fileNameSize = 32;
		char inputFilename[fileNameSize];
		createFoldersIfNeed(outputFolderName, "Histogram");

		std::string statisticsFilename = outputFolderName + "/Histogram/StatisticsHalf.csv";
		std::ofstream statisticsFile(statisticsFilename);

		std::vector<float> positionsCalcZ;
		std::vector<float> positionsTestZ;
		std::vector<float> imageMarkersCalc;
		std::vector<float> imageMarkersTest;
		for (size_t fileIndex = 0; fileIndex < positionsZ.size(); fileIndex++)
		{
			sprintf_s(inputFilename, fileNameSize, "Bright%4d.tif", (int)fileIndex);
			cv::Mat wideImage = cv::imread(imagesFolderName + "/" + inputFilename, cv::IMREAD_GRAYSCALE);

			std::string histogramFilename = outputFolderName + "/Histogram/HistogramHalf" +
				std::to_string(fileIndex) + ".csv";
			std::ofstream histogramFile(histogramFilename);

			float imageMarker = getImageMarker(wideImage, ImageMarkerType::GRAY_LEVEL_VARIANCE,
				histogramFile, statisticsFile);
			histogramFile.close();

			if (fileIndex % 2 == 0)
			{
				positionsCalcZ.push_back(positionsZ[fileIndex]);
				imageMarkersCalc.push_back(imageMarker);
			}
			else
			{
				positionsTestZ.push_back(positionsZ[fileIndex]);
				imageMarkersTest.push_back(imageMarker);
			}
		}

		statisticsFile.close();

		RegressionResult result = calculateRegression(imageMarkersCalc, positionsCalcZ);
		saveResults(positionsTestZ, imageMarkersTest, result, outputFolderName, true);
	}

private:
	float getImageMarker(cv::Mat& image, ImageMarkerType imageMarkerType,
		std::ofstream& histogramFile, std::ofstream& statisticsFile)
	{
		size_t rows = image.rows;
		size_t cols = image.cols;

		size_t rowCenterL = rows / 3;
		size_t rowCenterR = 2 * rows / 3;
		size_t colCenterL = cols / 3;
		size_t colCenterR = 2 * cols / 3;

		size_t pixelsNum = (rowCenterR - rowCenterL) * (colCenterR - colCenterL);

		size_t histogram[WHITE + 1];
		memset(histogram, 0, sizeof(histogram));

		for (size_t row = rowCenterL; row < rowCenterR; row++)
		{
			for (size_t col = colCenterL; col < colCenterR; col++)
			{
				byte val = image.at<byte>((int)row, (int)col);
				histogram[val]++;
			}
		}

		size_t modeGrayLevelIndex = 0;
		size_t modeGrayLevelValue = 0;
		size_t sum1GrayLevelValue = 0;
		size_t sum2GrayLevelValue = 0;
		for (size_t indexGrayLevel = BLACK; indexGrayLevel <= WHITE; indexGrayLevel++)
		{
			size_t valueGrayLevel = histogram[indexGrayLevel];
			if (valueGrayLevel > modeGrayLevelValue)
			{
				modeGrayLevelIndex = indexGrayLevel;
				modeGrayLevelValue = valueGrayLevel;
			}
			sum1GrayLevelValue += valueGrayLevel;
			sum2GrayLevelValue += valueGrayLevel * valueGrayLevel;
			histogramFile << valueGrayLevel << std::endl;
		}

		float expectation1 = (float)sum1GrayLevelValue / pixelsNum; // equals 1 by definition
		float expectation2 = (float)sum2GrayLevelValue / pixelsNum;
		float variance = expectation2 - expectation1 * expectation1;
		float standardDeviation = std::sqrtf(variance);

		statisticsFile <<
			modeGrayLevelIndex << "," <<
			modeGrayLevelValue << "," <<
			variance << "," <<
			standardDeviation << std::endl;

		return imageMarkerType == ImageMarkerType::GRAY_LEVEL_MODE ?
			(float)modeGrayLevelIndex : variance;
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
			+ (isHalf ? "Half" : "") + ".csv";
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
};
