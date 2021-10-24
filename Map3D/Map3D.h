#pragma once

#include "MapAPI.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"

Map map;
LayerScanner layerScanner;
std::vector<LayerInfo> layersWithCapillaries;
LayerInfo* bestLayerInfo; // can be nullptr if not enough capillaries found in all layers
CapillaryProcessor capillaryProcessor;

MAP_API void buildMap(const std::string& folderName)
{
	map.buildMap(folderName);
}

MAP_API void printValueAtTruncatedPos(float x, float y, float z)
{
	map.printValueAtTruncatedPos(x, y, z);
}

MAP_API void saveStiched(const std::string& outFolderName, bool withCorners)
{
#ifdef _DEBUG
	map.saveStiched(layersWithCapillaries, outFolderName, withCorners);
#endif
}

MAP_API void detectCapillaries(const std::string& outFolderName)
{
	layersWithCapillaries = layerScanner.detectCapillaries(map, outFolderName);
}

MAP_API void selectBestLayer()
{
	bestLayerInfo = layerScanner.selectBestLayer(layersWithCapillaries);
}

MAP_API void describeCapillaries(const std::string& outFolderName)
{
	if (bestLayerInfo != nullptr)
	{
		float startXmm = map.getStartXmm();
		float startYmm = map.getStartYmm();
		capillaryProcessor.describeCapillaries(startXmm, startYmm, map, *bestLayerInfo, outFolderName);
	}
}
