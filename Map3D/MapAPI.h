#pragma once

#ifdef MAP_EXPORT
#define MAP_API __declspec(dllexport)
#else
#define MAP_API __declspec(dllimport)
#endif
