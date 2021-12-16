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

std::string inputFolderNameMap;
std::string outputFolderNameMap;
std::string inputFolderNameLock;
std::string outputFolderNameLock;

void loadConfig(const char* configFilename)
{
	bool loadResult = config.load(configFilename);
	if (!loadResult)
	{
		std::cout << "Cannot load config file: " << configFilename << std::endl << std::endl;
	}
}

void initFolderNames()
{
	std::string inFolderNameMap			= config.getStringValue(keyInputMapFolder);
	std::string inDataFolderNameMap		= config.getStringValue(keyInputMapDataFolder);
	std::string inSubfolderNameMap		= config.getStringValue(keyInputMapSubfolder);

	std::string inFolderNameLock		= config.getStringValue(keyInputLockFolder);
	std::string inDataFolderNameLock	= config.getStringValue(keyInputLockDataFolder);
	std::string inSubfolderNameLock		= config.getStringValue(keyInputLockSubfolder);

	std::string outFolderName			= config.getStringValue(keyOutputFolder);

	inputFolderNameMap = inFolderNameMap + "/" + inDataFolderNameMap;
	outputFolderNameMap = outFolderName + "/" + inDataFolderNameMap;
	if (inSubfolderNameMap.size() > 0)
	{
		inputFolderNameMap += "/" + inSubfolderNameMap;
		outputFolderNameMap += "_" + inSubfolderNameMap; // avoid to create nested folders
	}

	inputFolderNameLock = inFolderNameLock + "/" + inDataFolderNameLock;
	outputFolderNameLock = outFolderName + "/" + inDataFolderNameLock;
	if (inSubfolderNameLock.size() > 0)
	{
		inputFolderNameLock += "/" + inSubfolderNameLock;
		outputFolderNameLock += "_" + inSubfolderNameLock; // avoid to create nested folders
	}
}

void initGeneralData()
{
	initGeneralData(config);
	initFolderNames();
}

void buildMap()
{
	map.buildMap(inputFolderNameMap, config);
}

void printValueAtTruncatedPos(float x, float y, float z)
{
	map.printValueAtTruncatedPos(x, y, z);
}

void saveStiched()
{
	map.saveStiched(layersWithCapillaries, outputFolderNameMap);
}

void detectCapillaries()
{
	layersWithCapillaries = layerScanner.detectCapillaries(map, outputFolderNameMap, config);
}

void describeCapillaries()
{
	if (layersWithCapillaries.empty())
	{
		std::cout << "No layers with enough capillaries are found" << std::endl << std::endl;
		return;
	}
#ifdef _DEBUG
	// Create and init file containing data of all layers
	std::string filenameAllLayers = outputFolderNameMap + "/Capillaries/ActualLayersFrames.csv";
	std::ofstream fileAllLayers(filenameAllLayers);
	fileAllLayers << "Layer,Frames,Max score,Sum score" << std::endl;
#endif
	capillaryProcessor.init(config);
	size_t bestLayerIndex = 0;
	float bestLayerSumScore = 0.0F;
	for (LayerInfo& layerInfo : layersWithCapillaries)
	{
		capillaryProcessor.describeCapillaries(map, layerInfo, outputFolderNameMap);
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
	std::string filenameSummary = outputFolderNameMap + "/Summary.txt";
	std::ofstream fileSummary(filenameSummary);
	fileSummary << "Best layer: " << bestLayerIndex + 1 << std::endl;
	fileSummary.close();
#ifdef _DEBUG
	fileAllLayers.close();
#endif
}

void loadPositionsZ()
{
	positionsZ = sequence.loadPositionsZ(inputFolderNameLock, config);
}

void buildSequence()
{
	sequence.buildSequence(inputFolderNameLock, config);
}

void saveProjections()
{
	sequence.saveProjections(outputFolderNameLock);
}

void calculateDepth()
{
	std::string focusingMethod = config.getStringValue(keyFocusingMethod);

	if (focusingMethod == "Mode")
	{
		wideImageProcessor.calculateStatistics(inputFolderNameLock, outputFolderNameLock,
			positionsZ, ImageMarkerType::GRAY_LEVEL_MODE, config);
	}

	if (focusingMethod == "Variance")
	{
		wideImageProcessor.calculateStatistics(inputFolderNameLock, outputFolderNameLock,
			positionsZ, ImageMarkerType::GRAY_LEVEL_VARIANCE, config);
	}

	if (focusingMethod == "Spectrum")
	{
		spectrumAnalyzer.calculateSpectrum(inputFolderNameLock, outputFolderNameLock, positionsZ);
	}
}
