#pragma once

#include "MapAPI.h"
#include "Config.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"
#include "Sequence.h"
#include "LineImageProcessor.h"
#include "WideImageProcessor.h"
#include "SpectrumAnalyzer.h"

MAP_API void loadConfig(std::string configFilename);
MAP_API void buildMap(const std::string& folderName);
MAP_API void printValueAtTruncatedPos(float x, float y, float z);
MAP_API void saveStiched(const std::string& outputFolderName);
MAP_API void detectCapillaries(const std::string& outputFolderName);
MAP_API void describeCapillaries(const std::string& outputFolderName);
MAP_API void loadPositionsZ(const std::string& folderName);
MAP_API void buildSequence(const std::string& folderName);
MAP_API void saveProjections(const std::string& outputFolderName);
MAP_API void calculateStatistics(const std::string& imagesFolderName, const std::string& outputFolderName);
MAP_API void calculateSpectrum(const std::string& imagesFolderName, const std::string& outputFolderName);
