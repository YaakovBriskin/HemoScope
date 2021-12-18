#pragma once

#include "MapAPI.h"
#include "LayerScanner.h"
#include "CapillaryProcessor.h"
#include "Sequence.h"
#include "LineImageProcessor.h"
#include "WideImageProcessor.h"
#include "SpectrumAnalyzer.h"

extern "C"
{
	MAP_API void __cdecl loadConfig(const char* configFilename);
	MAP_API void __cdecl initGeneralData();
	MAP_API void __cdecl buildMap();
	MAP_API void __cdecl printValueAtTruncatedPos(float x, float y, float z);
	MAP_API void __cdecl saveStiched();
	MAP_API void __cdecl detectCapillaries();
	MAP_API void __cdecl describeCapillaries();
	MAP_API void __cdecl loadPositionsZ();
	MAP_API void __cdecl buildSequence();
	MAP_API void __cdecl saveProjections();
	MAP_API void __cdecl calculateDepth();
	MAP_API void __cdecl overrideInt(const char* key, int val);
	MAP_API void __cdecl overrideFloat(const char* key, float val);
	MAP_API void __cdecl overrideString(const char* key, const char* val);
}