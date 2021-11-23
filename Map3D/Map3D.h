#pragma once

#include "MapAPI.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"
#include "Sequence.h"
#include "LineImageProcessor.h"
#include "WideImageProcessor.h"
#include "SpectrumAnalyzer.h"

Map map;
LayerScanner layerScanner;
std::vector<LayerInfo> layersWithCapillaries;
CapillaryProcessor capillaryProcessor;
LineImageProcessor lineImageProcessor;
WideImageProcessor wideImageProcessor;
SpectrumAnalyzer spectrumAnalyzer;
Sequence sequence;
std::vector<float> positionsZ;

MAP_API void buildMap(const std::string& folderName)
{
	map.buildMap(folderName);
}

MAP_API void printValueAtTruncatedPos(float x, float y, float z)
{
	map.printValueAtTruncatedPos(x, y, z);
}

MAP_API void saveStiched(const std::string& outputFolderName)
{
	map.saveStiched(layersWithCapillaries, outputFolderName);
}

MAP_API void detectCapillaries(const std::string& outputFolderName)
{
	layersWithCapillaries = layerScanner.detectCapillaries(map, outputFolderName);
}

MAP_API void describeCapillaries(const std::string& outputFolderName)
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
#ifdef _DEBUG
	fileAllLayers.close();
#endif
}

MAP_API void loadPositionsZ(const std::string& folderName)
{
	positionsZ = sequence.loadPositionsZ(folderName);
}

MAP_API void buildSequence(const std::string& folderName)
{
	sequence.buildSequence(folderName);
}

MAP_API void saveProjections(const std::string& outputFolderName)
{
	sequence.saveProjections(outputFolderName);
}

MAP_API void calculateStatistics(const std::string& imagesFolderName, const std::string& outputFolderName)
{
	wideImageProcessor.calculateStatistics(imagesFolderName, outputFolderName, positionsZ);
	wideImageProcessor.calculateStatisticsHalf(imagesFolderName, outputFolderName, positionsZ);
}

MAP_API void calculateSpectrum(const std::string& imagesFolderName, const std::string& outputFolderName)
{
	spectrumAnalyzer.calculateSpectrum(imagesFolderName, outputFolderName, positionsZ);
}
