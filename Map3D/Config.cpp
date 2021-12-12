#include "Config.h"

Config::Config()
{
}

bool Config::load(std::string configFilename)
{
	boost::property_tree::xml_parser::read_xml(configFilename, m_propertyTree);
	return true;
}

int Config::getIntValue(const std::string& key)
{
	return m_propertyTree.get<int>(key);
}

float Config::getFloatValue(const std::string& key)
{
	return m_propertyTree.get<float>(key);
}

std::string Config::getStringValue(const std::string& key)
{
	return m_propertyTree.get<std::string>(key);
}
