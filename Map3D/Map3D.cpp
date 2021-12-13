#include "Map3D.h"

Config config;
Map map;
LayerScanner layerScanner;
std::vector<LayerInfo> layersWithCapillaries;
CapillaryProcessor capillaryProcessor;
LineImageProcessor lineImageProcessor;
WideImageProcessor wideImageProcessor;
SpectrumAnalyzer spectrumAnalyzer;
Sequence sequence;
std::vector<float> positionsZ;

void loadConfig(std::string configFilename)
{
	bool loadResult = config.load(configFilename);
	if (!loadResult)
	{
		std::cout << "Cannot load config file: " << configFilename << std::endl << std::endl;
	}
}

void initGeneralData()
{
	initGeneralData(config);
}

void buildMap(const std::string& folderName)
{
	map.buildMap(folderName, config);
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
	layersWithCapillaries = layerScanner.detectCapillaries(map, outputFolderName, config);
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
	capillaryProcessor.init(config);
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
	positionsZ = sequence.loadPositionsZ(folderName, config);
}

void buildSequence(const std::string& folderName)
{
	sequence.buildSequence(folderName, config);
}

void saveProjections(const std::string& outputFolderName)
{
	sequence.saveProjections(outputFolderName);
}

void calculateDepth(const std::string& imagesFolderName, const std::string& outputFolderName)
{
	std::string focusingMethod = config.getStringValue(keyFocusingMethod);

	if (focusingMethod == "Mode")
	{
		wideImageProcessor.calculateStatistics(imagesFolderName, outputFolderName, positionsZ,
			ImageMarkerType::GRAY_LEVEL_MODE, config);
	}

	if (focusingMethod == "Variance")
	{
		wideImageProcessor.calculateStatistics(imagesFolderName, outputFolderName, positionsZ,
			ImageMarkerType::GRAY_LEVEL_VARIANCE, config);
	}

	if (focusingMethod == "Spectrum")
	{
		spectrumAnalyzer.calculateSpectrum(imagesFolderName, outputFolderName, positionsZ);
	}
}
