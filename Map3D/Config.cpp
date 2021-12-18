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
	if (m_overrides.contains(key))
	{
		return std::stoi(m_overrides[key]);
	}
	return m_propertyTree.get<int>(key);
}

float Config::getFloatValue(const std::string& key)
{
	if (m_overrides.contains(key))
	{
		return std::stof(m_overrides[key]);
	}
	return m_propertyTree.get<float>(key);
}

std::string Config::getStringValue(const std::string& key)
{
	if (m_overrides.contains(key))
	{
		return m_overrides[key];
	}
	return m_propertyTree.get<std::string>(key);
}

void Config::setOverride(const std::string& key, int val)
{
	m_overrides[key] = std::to_string(val);
}

void Config::setOverride(const std::string& key, float val)
{
	m_overrides[key] = std::to_string(val);
}

void Config::setOverride(const std::string& key, const std::string& val)
{
	m_overrides[key] = val;
}
