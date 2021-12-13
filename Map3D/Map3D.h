#pragma once

#include "MapAPI.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"
#include "Sequence.h"
#include "LineImageProcessor.h"
#include "WideImageProcessor.h"
#include "SpectrumAnalyzer.h"

MAP_API void loadConfig(std::string configFilename);
MAP_API void initGeneralData();
MAP_API void buildMap();
MAP_API void printValueAtTruncatedPos(float x, float y, float z);
MAP_API void saveStiched();
MAP_API void detectCapillaries();
MAP_API void describeCapillaries();
MAP_API void loadPositionsZ();
MAP_API void buildSequence();
MAP_API void saveProjections();
MAP_API void calculateDepth();
