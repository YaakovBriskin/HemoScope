#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <map>

const std::string keyPixelsInMm					= "HemoScope.General.PixelsInMm";
const std::string keyInputMapFolder				= "HemoScope.Input.Map.Folder";
const std::string keyInputLockFolder			= "HemoScope.Input.Lock.Folder";
const std::string keyOutputMapFolder			= "HemoScope.Output.Map.Folder";
const std::string keyOutputLockFolder			= "HemoScope.Output.Lock.Folder";
const std::string keyScanPosFilename			= "HemoScope.Procedures.Stitching.ScanPosFile";
const std::string keyMarkerCornerSize			= "HemoScope.Procedures.Stitching.MarkerCornerSize";
const std::string keyImageBiasPixelsX			= "HemoScope.Procedures.Stitching.Image.BiasPixels.X";
const std::string keyImageBiasPixelsY			= "HemoScope.Procedures.Stitching.Image.BiasPixels.Y";
const std::string keyImageMarginRelX			= "HemoScope.Procedures.Stitching.Image.MarginRelative.X";
const std::string keyImageMarginRelY			= "HemoScope.Procedures.Stitching.Image.MarginRelative.Y";
const std::string keyImageFrameRelW				= "HemoScope.Procedures.Stitching.Image.FrameRelative.Width";
const std::string keyImageFrameRelH				= "HemoScope.Procedures.Stitching.Image.FrameRelative.Height";
const std::string keyCroppedRows				= "HemoScope.Procedures.Identification.CroppedRows";
const std::string keyGrayLevelOriginalMin		= "HemoScope.Procedures.Identification.GrayLevelOriginal.Min";
const std::string keyGrayLevelOriginalMax		= "HemoScope.Procedures.Identification.GrayLevelOriginal.Max";
const std::string keyGradientThreshold			= "HemoScope.Procedures.Identification.GradientThreshold";
const std::string keyMinDistancePixels			= "HemoScope.Procedures.Identification.MinDistancePixels";
const std::string keyMinFoundCapillaries		= "HemoScope.Procedures.Identification.MinFoundCapillaries";
const std::string keyFineSmoothingKernelSize	= "HemoScope.Procedures.Characterization.FineSmoothingKernelSize";
const std::string keyDeepSmoothingKernelSize	= "HemoScope.Procedures.Characterization.DeepSmoothingKernelSize";
const std::string keyGrayLevelProcessedMin		= "HemoScope.Procedures.Characterization.GrayLevelProcessed.Min";
const std::string keyGrayLevelProcessedMax		= "HemoScope.Procedures.Characterization.GrayLevelProcessed.Max";
const std::string keyNumDescribedCappilaries	= "HemoScope.Procedures.Characterization.NumDescribedCappilaries";
const std::string keyMinPixelsInCappilary		= "HemoScope.Procedures.Characterization.MinPixelsInCappilary";
const std::string keySurroundingPixels			= "HemoScope.Procedures.Characterization.SurroundingPixels";
const std::string keyZPosFilename				= "HemoScope.Procedures.Focusing.ZPosFile";
const std::string keyFocusingMethod				= "HemoScope.Procedures.Focusing.Method";
const std::string keyModeImagePartCenter		= "HemoScope.Procedures.Focusing.Mode.ImagePartCenter";
const std::string keyVarianceImagePartCenter	= "HemoScope.Procedures.Focusing.Variance.ImagePartCenter";
const std::string keySpectrumSizeFFT			= "HemoScope.Procedures.Focusing.Spectrum.SizeFFT";
const std::string keySpectrumSizeEnergy			= "HemoScope.Procedures.Focusing.Spectrum.SizeEnergy";
const std::string keySpectrumNormalization		= "HemoScope.Procedures.Focusing.Spectrum.Normalization";

class Config
{
public:
	Config();
	bool load(std::string configFilename);

	int getIntValue(const std::string& key);
	float getFloatValue(const std::string& key);
	std::string getStringValue(const std::string& key);

	void setOverride(const std::string& key, int val);
	void setOverride(const std::string& key, float val);
	void setOverride(const std::string& key, const std::string& val);

private:
	boost::property_tree::ptree m_propertyTree;
	std::map<std::string, std::string> m_overrides;
};
