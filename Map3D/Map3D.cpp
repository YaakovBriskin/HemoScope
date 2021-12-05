#include "Map3D.h"

Map map;
LayerScanner layerScanner;
std::vector<LayerInfo> layersWithCapillaries;
CapillaryProcessor capillaryProcessor;
LineImageProcessor lineImageProcessor;
WideImageProcessor wideImageProcessor;
SpectrumAnalyzer spectrumAnalyzer;
Sequence sequence;
std::vector<float> positionsZ;

void buildMap(const std::string& folderName)
{
	map.buildMap(folderName);
}

void printValueAtTruncatedPos(float x, float y, float z)
{
	map.printValueAtTruncatedPos(x, y, z);
}

void saveStiched(const std::string& outputFolderName)
{
	map.saveStiched(layersWithCapillaries, outputFolderName);
}

void detectCapillaries(const std::string& outputFolderName)
{
	layersWithCapillaries = layerScanner.detectCapillaries(map, outputFolderName);
}

void describeCapillaries(const std::string& outputFolderName)
{
	if (layersWithCapillaries.empty())
	{
		std::cout << "No layers with enough capillaries are found" << std::endl << std::endl;
		return;
	}
#ifdef _DEBUG
	// Create and init file containing data of all layers
	std::string filenameAllLayers = outputFolderName + "/Capillaries/ActualLayersFrames.csv";
	std::ofstream fileAllLayers(filenameAllLayers);
	fileAllLayers << "Layer,Frames,Max score,Sum score" << std::endl;
#endif
	size_t bestLayerIndex = 0;
	float bestLayerSumScore = 0.0F;
	for (LayerInfo& layerInfo : layersWithCapillaries)
	{
		capillaryProcessor.describeCapillaries(map, layerInfo, outputFolderName);
#ifdef _DEBUG
		std::string printedLine =
			std::to_string(layerInfo.layerIndex + 1) + "," +
			std::to_string(layerInfo.capillariesInfo.size()) + "," +
			toString(layerInfo.maxScore, 1) + "," +
			toString(layerInfo.sumScore, 1);
		fileAllLayers << printedLine << std::endl;
#endif
		if (layerInfo.sumScore > bestLayerSumScore)
		{
			bestLayerIndex = layerInfo.layerIndex;
			bestLayerSumScore = layerInfo.sumScore;
		}
	}
	std::cout << "Best layer: " << bestLayerIndex + 1 << std::endl << std::endl;
	std::string filenameSummary = outputFolderName + "/Summary.txt";
	std::ofstream fileSummary(filenameSummary);
	fileSummary << "Best layer: " << bestLayerIndex + 1 << std::endl;
	fileSummary.close();
#ifdef _DEBUG
	fileAllLayers.close();
#endif
}

void loadPositionsZ(const std::string& folderName)
{
	positionsZ = sequence.loadPositionsZ(folderName);
}

void buildSequence(const std::string& folderName)
{
	sequence.buildSequence(folderName);
}

void saveProjections(const std::string& outputFolderName)
{
	sequence.saveProjections(outputFolderName);
}

void calculateStatistics(const std::string& imagesFolderName, const std::string& outputFolderName)
{
	wideImageProcessor.calculateStatistics(imagesFolderName, outputFolderName, positionsZ);
	//wideImageProcessor.calculateStatisticsHalf(imagesFolderName, outputFolderName, positionsZ);
}

void calculateSpectrum(const std::string& imagesFolderName, const std::string& outputFolderName)
{
	spectrumAnalyzer.calculateSpectrum(imagesFolderName, outputFolderName, positionsZ);
}
