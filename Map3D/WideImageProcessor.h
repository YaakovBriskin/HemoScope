#pragma once


#include <vector>
#include <fstream>

#include "Sequence.h"

class WideImageProcessor
{
public:
	void calculateGradient(std::vector<Projection>& projections, const std::string& outputFolderName)
	{
		createFoldersIfNeed(outputFolderName, "Gradients");
		size_t fileIndex = 0;
		for (Projection projection : projections)
		{
			ByteMatrix gradMatrix(projection.wideMatrix.rows(), projection.wideMatrix.cols());
			calculateGradient(projection.wideMatrix, gradMatrix);
			std::string gradFilename = outputFolderName + "/Gradients/WideGrad" +
				std::to_string(fileIndex++) + ".bmp";
			bool result = cv::imwrite(gradFilename, gradMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + gradFilename).c_str());
			}
		}
	}

	void calculateExcess(std::vector<Projection>& projections, const std::string& outputFolderName)
	{
		createFoldersIfNeed(outputFolderName, "Excess");
		size_t fileIndex = 0;
		for (Projection projection : projections)
		{
			ByteMatrix gradMatrix(projection.wideMatrix.rows(), projection.wideMatrix.cols());

			std::string histogramFilename = outputFolderName + "/Excess/Histogram" +
				std::to_string(fileIndex) + ".csv";
			std::ofstream histogramFile(histogramFilename);

			calculateExcess(projection.wideMatrix, gradMatrix, histogramFile);
			histogramFile.close();

			std::string spectrumFilename = outputFolderName + "/Excess/HighFreq" +
				std::to_string(fileIndex) + ".bmp";
			bool result = cv::imwrite(spectrumFilename, gradMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + spectrumFilename).c_str());
			}

			fileIndex++;
		}
	}

	void calculateSpectrum(std::vector<Projection>& projections, const std::string& outputFolderName)
	{
		createFoldersIfNeed(outputFolderName, "Spectrum");
		size_t fileIndex = 0;
		for (Projection projection : projections)
		{
			ByteMatrix gradMatrix(projection.wideMatrix.rows(), projection.wideMatrix.cols());
			calculateFourierTransform(projection.wideMatrix, gradMatrix);
			std::string spectrumFilename = outputFolderName + "/Spectrum/DFT" +
				std::to_string(fileIndex++) + ".bmp";
			bool result = cv::imwrite(spectrumFilename, gradMatrix.asCvMatU8());
			if (!result)
			{
				throw std::exception(("Cannot write file: " + spectrumFilename).c_str());
			}
		}
	}

private:
	void calculateGradient(ByteMatrix& src, ByteMatrix& dst)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		for (size_t row = 0; row < rows; row++)
		{
			for (size_t col = 0; col < cols; col++)
			{
				if ((row == 0) || (row == rows - 1) || (col == 0) || (col == cols - 1))
				{
					dst.set(row, col, 0);
					continue;
				}

				byte valPrevRow = src.get(row - 1, col);
				byte valNextRow = src.get(row + 1, col);
				float gradRow = 10.0F * ((float)valNextRow - (float)valPrevRow);

				byte valPrevCol = src.get(row, col - 1);
				byte valNextCol = src.get(row, col + 1);
				float gradCol = 10.0F * ((float)valNextCol - (float)valPrevCol);

				byte grad = (byte)std::fminf(std::roundf(std::sqrtf(gradRow * gradRow + gradCol * gradCol)), 255.0F);
				dst.set(row, col, grad);
			}
		}
	}

	void calculateExcess(ByteMatrix& src, ByteMatrix& dst, std::ofstream& histogramFile)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();

		const size_t halfKernelSize = DEEP_SMOOTHING_KERNEL_SIZE / 2;

		size_t histogram[256];
		memset(histogram, 0, sizeof(histogram));

		for (size_t row = 0; row < rows; row++)
		{
			for (size_t col = 0; col < cols; col++)
			{
				if ((row < halfKernelSize) || (row >= rows - halfKernelSize) ||
					(col < halfKernelSize) || (col >= cols - halfKernelSize))
				{
					dst.set(row, col, 0);
					continue;
				}

				// Calculate excess over blurred
				unsigned int sum = 0;
				for (size_t kernelRow = row - halfKernelSize; kernelRow <= row + halfKernelSize; kernelRow++)
				{
					for (size_t kernelCol = col - halfKernelSize; kernelCol <= col + halfKernelSize; kernelCol++)
					{
						sum += src.get(kernelRow, kernelCol);
					}
				}
				float blurred = (float)sum / DEEP_SMOOTHING_KERNEL_SIZE / DEEP_SMOOTHING_KERNEL_SIZE;
				float excess = 2.0F * (src.get(row, col) / blurred - 0.75F);
				excess = std::fminf(std::fmaxf(0.0F, excess), 1.0F);
				byte normalizedExcess = (byte)std::roundf(WHITE * excess);
				dst.set(row, col, normalizedExcess);
				histogram[normalizedExcess]++;
			}
		}

		for (size_t i = 0; i < 256; i++)
		{
			histogramFile << histogram[i] << std::endl;
		}
	}

	// EXTREMELY SLOW IN CURRENT IMPLEMENTATION
	void calculateFourierTransform(ByteMatrix& src, ByteMatrix& dst)
	{
		size_t rows = src.rows();
		size_t cols = src.cols();
		float size = std::sqrtf((float)rows * (float)cols);

		for (size_t u = 0; u < rows; u++)
		{
			for (size_t v = 0; v < cols; v++)
			{
				float re = 0.0F;
				float im = 0.0F;
				for (size_t row = 0; row < rows; row++)
				{
					for (size_t col = 0; col < cols; col++)
					{
						float factorRow = 2.0F * (float)std::numbers::pi * u * row / (float)rows;
						float factorCol = 2.0F * (float)std::numbers::pi * v * col / (float)cols;
						float val = (float)src.get(row, col);
						re += val * std::cosf(factorRow + factorCol) / size;
						im -= val * std::sinf(factorRow + factorCol) / size;
					}
				}
				byte amp = (byte)std::fminf(std::roundf(std::sqrtf(re * re + im * im)), 255.0F);
				dst.set(u, v, amp);
			}
		}
	}
};
