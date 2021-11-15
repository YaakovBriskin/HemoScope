#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable: 5054)
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#pragma warning(pop)

#include "ByteMatrix.h"

const std::string Z_POS_FILENAME = "TF_vec_col.csv";

class Projection
{
public:
	ByteMatrix wideMatrix;
	ByteMatrix lineMatrix;
	float z;
	Projection(const ByteMatrix& wideMat, const ByteMatrix& lineMat, float posZ)
	{
		wideMatrix = wideMat;
		lineMatrix = lineMat;
		z = posZ;
	}
};

class Sequence
{
public:
	Sequence()
	{
	}

	void buildSequence(const std::string& folderName)
	{
		std::filesystem::path folder = std::filesystem::absolute(std::filesystem::path(folderName));
		std::string absFolderName = folder.generic_string();
		std::replace(absFolderName.begin(), absFolderName.end(), '/', '\\');
		std::cout << "Input data folder:" << std::endl << absFolderName << std::endl << std::endl;

		const size_t fileNameSize = 32;
		char wideFilename[fileNameSize];
		char lineFilename[fileNameSize];
		size_t fileIndex = 0;

		// Open file with Z positions
		std::string zPosPathFilename = folderName + "/" + Z_POS_FILENAME;
		std::ifstream zPosFile(zPosPathFilename);

		// Iterate Z positions in the file and fill the vector of projections
		std::string line;
		while (!zPosFile.eof())
		{
			getline(zPosFile, line);
			if (line.empty())
			{
				break;
			}
			float z = (float)atof(line.c_str());

			sprintf_s(wideFilename, fileNameSize, "Bright%4d.tif", (int)fileIndex);
			cv::Mat wideImage = cv::imread(folderName + "/" + wideFilename, cv::IMREAD_GRAYSCALE);
			ByteMatrix wideMatrix(wideImage.rows, wideImage.cols);
			image2Matrix(wideImage, wideMatrix, false);

			sprintf_s(lineFilename, fileNameSize, "Line%4d.tif", (int)fileIndex);
			cv::Mat lineImage = cv::imread(folderName + "/" + lineFilename, cv::IMREAD_GRAYSCALE);
			ByteMatrix lineMatrix(lineImage.rows, lineImage.cols);
			image2Matrix(lineImage, lineMatrix, true);

			m_projections.push_back(Projection(wideMatrix, lineMatrix, z));

			fileIndex++;
		}

		zPosFile.close();
	}

	void saveProjections(const std::string& outFolderName)
	{
		createFoldersIfNeed(outFolderName, "Projections");

		size_t fileIndex = 0;
		bool result = true;
		for (Projection& projection : m_projections)
		{
			std::string wideFilename = outFolderName + "/Projections/Wide" +
				std::to_string(fileIndex) + ".bmp";
			result = cv::imwrite(wideFilename, projection.wideMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + wideFilename).c_str());
			}

			std::string lineFilename = outFolderName + "/Projections/Line" +
				std::to_string(fileIndex) + ".bmp";
			result = cv::imwrite(lineFilename, projection.lineMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + lineFilename).c_str());
			}

			fileIndex++;
		}
	}

	std::vector<Projection> getProjections()
	{
		return m_projections;
	}

private:
	std::vector<Projection> m_projections;

private:
	void image2Matrix(const cv::Mat& srcImage, ByteMatrix& dstMatrix, bool makeContrast)
	{
		for (size_t row = 0; row < srcImage.rows; row++)
		{
			for (size_t col = 0; col < srcImage.cols; col++)
			{
				byte val = srcImage.at<byte>((int)row, (int)col);
				if (makeContrast)
				{
					val = val <= DELIM ? BLACK : WHITE;
				}
				dstMatrix.set(row, col, val);
			}
		}
	}
};
