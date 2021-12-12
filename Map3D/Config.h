#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

const std::string keyScanPosFilename	= "HemoScope.Procedures.Stitching.ScanPosFile";
const std::string keyMarkerCornerSize	= "HemoScope.Procedures.Stitching.MarkerCornerSize";
const std::string keyImageBiasPixelsX	= "HemoScope.Procedures.Stitching.Image.BiasPixels.X";
const std::string keyImageBiasPixelsY	= "HemoScope.Procedures.Stitching.Image.BiasPixels.Y";
const std::string keyImageMarginRelX	= "HemoScope.Procedures.Stitching.Image.MarginRelative.X";
const std::string keyImageMarginRelY	= "HemoScope.Procedures.Stitching.Image.MarginRelative.Y";
const std::string keyImageFrameRelW		= "HemoScope.Procedures.Stitching.Image.FrameRelative.Width";
const std::string keyImageFrameRelH		= "HemoScope.Procedures.Stitching.Image.FrameRelative.Height";

class Config
{
public:
	Config();
	bool load(std::string configFilename);

	int getIntValue(const std::string& key);
	float getFloatValue(const std::string& key);
	std::string getStringValue(const std::string& key);

private:
	boost::property_tree::ptree m_propertyTree;
};
