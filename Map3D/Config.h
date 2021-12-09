#pragma once

#include <string>

class Config
{
public:
	Config();
	bool load(std::string configFilename);
};
