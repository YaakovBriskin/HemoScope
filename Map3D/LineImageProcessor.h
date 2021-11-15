#pragma once

#include <vector>
#include <fstream>

#include "Sequence.h"

class LineImageProcessor
{
public:
	void calculateGradient(std::vector<Projection>& projections, const std::string& outFolderName)
	{
		createFoldersIfNeed(outFolderName, "Gradients");
		size_t fileIndex = 0;
		for (Projection projection : projections)
		{
			ByteMatrix gradMatrix(projection.lineMatrix.rows(), projection.lineMatrix.cols());
			calculateGradientY(projection.lineMatrix, gradMatrix);
			std::string gradFilename = outFolderName + "/Gradients/LineGrad" +
				std::to_string(fileIndex++) + ".bmp";
			bool result = cv::imwrite(gradFilename, gradMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + gradFilename).c_str());
			}
		}
	}

	void calculateStatistics(std::vector<Projection>& projections, const std::string& outFolderName)
	{
		createFoldersIfNeed(outFolderName, "Statistics");
		size_t fileIndex = 0;
		for (Projection projection : projections)
		{
			std::string whiteInColsFilename = outFolderName + "/Statistics/WhiteInCols" +
				std::to_string(fileIndex++) + ".csv";
			std::ofstream whiteInColsFile(whiteInColsFilename);
			countWhitePixels(projection.lineMatrix, whiteInColsFile);
			whiteInColsFile.close();
		}
	}

private:
	void calculateGradientY(ByteMatrix& lineMatrix, ByteMatrix& gradMatrix)
	{
		size_t rows = lineMatrix.rows();
		size_t cols = lineMatrix.cols();

		for (size_t row = 0; row < rows; row++)
		{
			for (size_t col = 0; col < cols; col++)
			{
				if ((row == 0) || (row == rows - 1))
				{
					gradMatrix.set(row, col, 0);
					continue;
				}
				byte valPrevRow = lineMatrix.get(row - 1, col);
				byte valNextRow = lineMatrix.get(row + 1, col);
				byte valGradRow = valPrevRow == valNextRow ? BLACK : WHITE;
				gradMatrix.set(row, col, valGradRow);
			}
		}
	}

	void countWhitePixels(ByteMatrix& matrix, std::ofstream& whiteInColsFile)
	{
		size_t rows = matrix.rows();
		size_t cols = matrix.cols();

		std::vector<size_t> whitePixels;
		for (size_t col = 0; col < cols; col++)
		{
			size_t sumInCol = 0;
			for (size_t row = 0; row < rows; row++)
			{
				sumInCol += matrix.get(row, col);
			}
			whitePixels.push_back(sumInCol / WHITE);
		}

		const size_t smoothingOrder = 100;
		const float smoothingDepth = 0.99F;
		float weight = 1.0F;
		std::vector<float> weights;
		float sumWeights = 0.0F;
		for (size_t i = 0; i <= smoothingOrder; i++)
		{
			weight *= smoothingDepth;
			weights.push_back(weight);
			sumWeights += weight;
		}

		for (size_t col = 0; col < cols; col++)
		{
			float smothed = 0.0F;
			if (col >= smoothingOrder)
			{
				for (size_t i = 0; i <= smoothingOrder; i++)
				{
					smothed += weights[i] * whitePixels[col - i];
				}
				smothed /= sumWeights;
			}
			whiteInColsFile << col << "," << whitePixels[col] << "," << smothed << std::endl;
		}
	}
};
