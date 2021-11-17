#pragma once

#include <vector>
#include <numeric>

#include "Map.h"
#include "CornerDetector.h"

class LayerScanner
{
public:
	std::vector<LayerInfo> detectCapillaries(Map& map, const std::string& outputFolderName)
	{
		std::cout << "Detection of capillaries started" << std::endl << std::endl;
		m_timer.start();

		// Create folder for capillaries data
		std::string capillariesFolderName = outputFolderName + "/Capillaries";
#ifdef _DEBUG
		if (!std::filesystem::exists(std::filesystem::path(capillariesFolderName)))
		{
			bool result = std::filesystem::create_directory(std::filesystem::path(capillariesFolderName));
			if (!result)
			{
				throw std::exception(("Cannot create folder: " + capillariesFolderName).c_str());
			}
		}

		// Create and init file containing data of all layers
		std::string filenameAllLayers = capillariesFolderName + "/AllLayersDetection.csv";
		std::ofstream fileAllLayers(filenameAllLayers);
		fileAllLayers << "Layer,Corners,Max score,Sum score" << std::endl;
#endif
		std::vector<LayerInfo> layersWithCapillaries;

		// For each layer
		std::vector<Layer> layers = map.getLayers();
		for (size_t layerIndex = 0; layerIndex < layers.size(); layerIndex++)
		{
			// Find corners in the layer and score them depending on stand out from the background
			Layer layer = layers[layerIndex];
			m_cornerDetector.setLayerPosition(layer.z);
			std::vector<ScoredCorner> scoredCorners = m_cornerDetector.getCornersSobel(map, layer.matrix,
				capillariesFolderName, layerIndex);

			// Update layer info by detected corners of capillaries
			LayerInfo layerInfo;
			layerInfo.layerIndex = layerIndex;
			layerInfo.z = layer.z;
			layerInfo.capillaryApexes = scoredCorners;
			layerInfo.maxScore = scoredCorners.size() > 0 ? scoredCorners.begin()->score : 0;
			layerInfo.sumScore = std::accumulate(scoredCorners.begin(), scoredCorners.end(), 0.0F,
				[](float sum, const ScoredCorner& corner) { return sum + corner.score; });

#ifdef _DEBUG
			std::cout <<
				"Layer:     " << layerIndex + 1 << std::endl <<
				"Corners:   " << scoredCorners.size() << std::endl <<
				"Max score: " << std::setprecision(6) << layerInfo.maxScore << std::endl <<
				"Sum score: " << std::setprecision(6) << layerInfo.sumScore << std::endl << std::endl;

			std::string printedLine =
				std::to_string(layerIndex + 1) + "," +
				std::to_string(scoredCorners.size()) + "," +
				toString(layerInfo.maxScore, 1) + "," +
				toString(layerInfo.sumScore, 1);
			fileAllLayers << printedLine << std::endl;
#endif
			if ((scoredCorners.size() >= MIN_FOUND_CAPILLARIES) && (layerInfo.sumScore > 0.0F))
			{
				layersWithCapillaries.push_back(layerInfo);
			}
			else
			{
				std::cout << "Not enough capillaries found in layer " << layerIndex + 1 << std::endl << std::endl;
			}
		}

#ifdef _DEBUG
		fileAllLayers.close();
#endif
		m_timer.end();
		std::cout << "Detection of capillaries completed in " <<
			m_timer.getDurationMilliseconds() << " ms" << std::endl << std::endl;

		return layersWithCapillaries;
	}

private:
	CornerDetector m_cornerDetector;
	Timer m_timer;
};
