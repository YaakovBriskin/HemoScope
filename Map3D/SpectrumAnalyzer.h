#pragma once

#include <vector>
#include <complex>

#include "../simple_fft/fft_settings.h"
#include "../simple_fft/fft.h"
#include "ByteMatrix.h"

#pragma warning(disable: 26812)

typedef std::vector<std::vector<real_type>> RealArray2D;
typedef std::vector<std::vector<complex_type>> ComplexArray2D;

const size_t FFT_SIZE = 512;
const size_t ENERGY_SIZE = 25;
const double NORMALIZATIUON = 15.0;

enum AreaType
{
	NOTHING,
	CORNERS,
	CENTRAL
};

class SpectrumAnalyzer
{
public:
	RegressionResult calculateSpectrum(const std::string& imagesFolderName, const std::string& outputFolderName,
		std::vector<float>& positionsZ)
	{
		std::filesystem::path inputFolder = std::filesystem::absolute(std::filesystem::path(imagesFolderName));
		std::string absFolderName = inputFolder.generic_string();
		std::replace(absFolderName.begin(), absFolderName.end(), '/', '\\');
		std::cout << "Input data folder:" << std::endl << absFolderName << std::endl << std::endl;

		const size_t fileNameSize = 32;
		char inputFilename[fileNameSize];
		createFoldersIfNeed(outputFolderName, "SpectrumFFT");

		std::vector<float> energyValues;
		for (size_t fileIndex = 0; fileIndex < positionsZ.size(); fileIndex++)
		{
			sprintf_s(inputFilename, fileNameSize, "Bright%4d.tif", (int)fileIndex);
			cv::Mat wideImage = cv::imread(imagesFolderName + "/" + inputFilename, cv::IMREAD_GRAYSCALE);

			m_rows = (size_t)wideImage.rows;
			m_cols = (size_t)wideImage.cols;
			cv::Mat wideSpectrum(wideImage.rows, wideImage.cols, CV_8U);

			float energyDiff = calculateFFT(wideImage, wideSpectrum);
			energyValues.push_back(energyDiff);

			std::string spectrumFilename = outputFolderName + "/SpectrumFFT/Spectrum" +
				std::to_string(fileIndex) + ".bmp";
			bool result = cv::imwrite(spectrumFilename, wideSpectrum);
			if (!result)
			{
				throw std::exception(("Cannot write file: " + spectrumFilename).c_str());
			}
		}

		RegressionResult result = calculateRegression(energyValues, positionsZ);
		saveResults(positionsZ, energyValues, result, outputFolderName);
		return result;
	}

private:
	size_t m_rows;
	size_t m_cols;

private:
	float calculateFFT(cv::Mat& src, cv::Mat& dst)
	{
		size_t rowBegin = (FFT_SIZE - m_rows) / 2;
		size_t colBegin = (FFT_SIZE - m_cols) / 2;

		RealArray2D matSrc;
		ComplexArray2D matDst;
		const char* errMsg = nullptr;

		for (size_t row = 0; row < FFT_SIZE; row++)
		{
			// Add empty destination row
			std::vector<complex_type> rowDst;
			rowDst.resize(FFT_SIZE);
			matDst.push_back(rowDst);

			// Source row: empty to pad for FFT size or contains source image data
			std::vector<real_type> rowSrc;
			rowSrc.resize(FFT_SIZE);

			// Add padding row of source if index of the row is out of image rows
			if ((row < rowBegin) || (row >= rowBegin + m_rows))
			{
				matSrc.push_back(rowSrc);
				continue;
			}

			// Fill and add source row that contains pixels of the image
			for (size_t col = colBegin; col < colBegin + m_cols; col++)
			{
				byte val = src.at<byte>((int)(row - rowBegin), (int)(col - colBegin));
				rowSrc[col] = (real_type)val;
			}
			matSrc.push_back(rowSrc);
		}

		bool result = simple_fft::FFT<RealArray2D, ComplexArray2D>(matSrc, matDst, FFT_SIZE, FFT_SIZE, errMsg);
		if (!result)
		{
			std::cout << "FFT: " << errMsg << std::endl << std::endl;
			return 0.0F;
		}

		float energyCorners = 0.0F;
		float energyCentral = 0.0F;
		for (size_t row = rowBegin; row < rowBegin + m_rows; row++)
		{
			std::vector<complex_type> rowDst = matDst[row];
			for (size_t col = colBegin; col < colBegin + m_cols; col++)
			{
				complex_type val = rowDst[col];
				real_type valReal = std::round(std::abs(val.real())) / NORMALIZATIUON;
				real_type valNorm = std::min(valReal, 255.0);
				dst.at<byte>((int)(row - rowBegin), (int)(col - colBegin)) = (byte)valNorm;

				AreaType areaType = getAreaType(row - rowBegin, col - colBegin);
				if (areaType == AreaType::CORNERS)
				{
					energyCorners += (float)valNorm;
				}
				if (areaType == AreaType::CENTRAL)
				{
					energyCentral += (float)valNorm;
				}
			}
		}

		float energyDiff = (energyCorners - energyCentral) / 4.0F / ENERGY_SIZE / ENERGY_SIZE;
		return std::fmaxf(0.0F, energyDiff);
	}

	AreaType getAreaType(size_t row, size_t col)
	{
		bool rowInCornersArea = (row < ENERGY_SIZE) || (row >= m_rows - ENERGY_SIZE);
		bool colInCornersArea = (col < ENERGY_SIZE) || (col >= m_cols - ENERGY_SIZE);
		if (rowInCornersArea && colInCornersArea)
		{
			return AreaType::CORNERS;
		}

		bool rowInCentralArea = std::abs((int)row - (int)m_rows / 2) < ENERGY_SIZE;
		bool colInCentralArea = std::abs((int)col - (int)m_cols / 2) < ENERGY_SIZE;
		if (rowInCentralArea && colInCentralArea)
		{
			return AreaType::CENTRAL;
		}

		return AreaType::NOTHING;
	}

	RegressionResult calculateRegression(std::vector<float>& energyValues, std::vector<float>& positionsZ)
	{
		size_t n = energyValues.size();
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
			sumX1 += energyValues[i];
			sumX2 += energyValues[i] * energyValues[i];
			sumY1 += positionsZ[i];
			sumXY += energyValues[i] * positionsZ[i];
		}

		RegressionResult result;
		result.slope = (n * sumXY - sumX1 * sumY1) / (n * sumX2 - sumX1 * sumX1);
		result.offset = (sumX2 * sumY1 - sumXY * sumX1) / (n * sumX2 - sumX1 * sumX1);
		return result;
	}

	void saveResults(std::vector<float>& positionsZ, std::vector<float>& energyValues,
		RegressionResult& result, const std::string& outputFolderName)
	{
		std::string positionsFilename = outputFolderName + "/PositionsZ.csv";
		std::ofstream positionsFile(positionsFilename);
		positionsFile << "Index,Z given,Z calculated" << std::endl;

		for (size_t posIndex = 0; posIndex < positionsZ.size(); posIndex++)
		{
			float positionCalc = result.slope * energyValues[posIndex] + result.offset;
			positionsFile <<
				posIndex << "," <<
				positionsZ[posIndex] << "," <<
				positionCalc << std::endl;
		}

		positionsFile.close();
	}
};
