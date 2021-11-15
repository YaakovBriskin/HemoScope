#pragma once

#include "MapAPI.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"
#include "Sequence.h"
#include "LineImageProcessor.h"
#include "WideImageProcessor.h"

Map map;
LayerScanner layerScanner;
std::vector<LayerInfo> layersWithCapillaries;
CapillaryProcessor capillaryProcessor;
Sequence sequence;
LineImageProcessor lineImageProcessor;
WideImageProcessor wideImageProcessor;

MAP_API void buildMap(const std::string& folderName)
{
	map.buildMap(folderName);
}

MAP_API void printValueAtTruncatedPos(float x, float y, float z)
{
	map.printValueAtTruncatedPos(x, y, z);
}

MAP_API void saveStiched(const std::string& outFolderName)
{
	map.saveStiched(layersWithCapillaries, outFolderName);
}

MAP_API void detectCapillaries(const std::string& outFolderName)
{
	layersWithCapillaries = layerScanner.detectCapillaries(map, outFolderName);
}

MAP_API void describeCapillaries(const std::string& outFolderName)
{
	if (layersWithCapillaries.empty())
	{
		std::cout << "No layers with enough capillaries are found" << std::endl << std::endl;
		return;
	}
#ifdef _DEBUG
	// Create and init file containing data of all layers
	std::string filenameAllLayers = outFolderName + "/Capillaries/ActualLayersFrames.csv";
	std::ofstream fileAllLayers(filenameAllLayers);
	fileAllLayers << "Layer,Frames,Max score,Sum score" << std::endl;
#endif
	size_t bestLayerIndex = 0;
	float bestLayerSumScore = 0.0F;
	for (LayerInfo& layerInfo : layersWithCapillaries)
	{
		capillaryProcessor.describeCapillaries(map, layerInfo, outFolderName);
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

MAP_API void buildSequence(const std::string& folderName)
{
	sequence.buildSequence(folderName);
}

MAP_API void saveProjections(const std::string& outFolderName)
{
	sequence.saveProjections(outFolderName);
}

MAP_API void calculateGradient(const std::string& outFolderName)
{
	std::vector<Projection> projections = sequence.getProjections();
	//lineImageProcessor.calculateGradient(projections, outFolderName);
	wideImageProcessor.calculateGradient(projections, outFolderName);
}

MAP_API void calculateExcess(const std::string& outFolderName)
{
	std::vector<Projection> projections = sequence.getProjections();
	wideImageProcessor.calculateExcess(projections, outFolderName);
}

MAP_API void calculateSpectrum(const std::string& outFolderName)
{
	std::vector<Projection> projections = sequence.getProjections();
	wideImageProcessor.calculateSpectrum(projections, outFolderName);
}

MAP_API void calculateStatistics(const std::string& outFolderName)
{
	std::vector<Projection> projections = sequence.getProjections();
	lineImageProcessor.calculateStatistics(projections, outFolderName);
}
