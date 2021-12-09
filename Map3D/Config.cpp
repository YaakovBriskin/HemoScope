#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Config.h"

Config::Config()
{

}

bool Config::load(std::string configFilename)
{
	boost::property_tree::ptree props;
	boost::property_tree::xml_parser::read_xml(configFilename, props);
	float imageMarginRelativeX = props.get<float>("HemoScope.Procedures.Stitching.Image.MarginRate.X");
	float imageMarginRelativeY = props.get<float>("HemoScope.Procedures.Stitching.Image.MarginRate.Y");
	return true;
}
