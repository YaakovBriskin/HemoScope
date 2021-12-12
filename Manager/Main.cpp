#include <iostream>
#include "Map3D.h"

void processCapillaries()
{
    const std::string inputFolderName = "../XYZ";
    const std::string dataFolderName = "20";
    const std::string subfolderName = "";
    std::string imagesFolderName = inputFolderName + "/" + dataFolderName;
    std::string outputFolderName = "../Output/" + dataFolderName;
    if (subfolderName.size() > 0)
    {
        imagesFolderName += "/" + subfolderName;
        outputFolderName += "_" + subfolderName; // avoid to create nested folders
    }

    buildMap(imagesFolderName);
    saveStiched(outputFolderName);
    printValueAtTruncatedPos(400.0F, 200.0F, 24.6F); // example of map usage
    detectCapillaries(outputFolderName);
    describeCapillaries(outputFolderName);
    saveStiched(outputFolderName);
}

void processFocus()
{
    const std::string inputFolderName = "../Z_locking";
    const std::string dataFolderName = "ThroughFocus_C11_2";
    std::string imagesFolderName = inputFolderName + "/" + dataFolderName;
    std::string outputFolderName = "../Output/" + dataFolderName;
    loadPositionsZ(imagesFolderName);
    calculateStatistics(imagesFolderName, outputFolderName);
    //calculateSpectrum(imagesFolderName, outputFolderName);
}

int main()
{
    loadConfig("../Config/Config.xml");
    processCapillaries();
    //processFocus();
    return 0;
}
