#include <iostream>
#include "Map3D.h"

void processCapillaries()
{
    const std::string inputFolderName = "../ScansHiRes";
    const std::string dataFolderName = "emc11";
    const std::string subfolderName = "5";
    std::string imagesFolderName = inputFolderName + "/" + dataFolderName;
    std::string outFolderName = "../Output/" + dataFolderName;
    if (subfolderName.size() > 0)
    {
        imagesFolderName += "/" + subfolderName;
        outFolderName += "_" + subfolderName; // avoid to create nested folders
    }

    buildMap(imagesFolderName);
    saveStiched(outFolderName);
    printValueAtTruncatedPos(400.0F, 200.0F, 24.6F); // example of map usage
    detectCapillaries(outFolderName);
    describeCapillaries(outFolderName);
    saveStiched(outFolderName);
}

void processFocus()
{
    const std::string inputFolderName = "../Z_locking";
    const std::string dataFolderName = "ThroughFocus_C11_2";
    std::string imagesFolderName = inputFolderName + "/" + dataFolderName;
    std::string outFolderName = "../Output/" + dataFolderName;
    buildSequence(imagesFolderName);
    saveProjections(outFolderName);
    calculateExcess(outFolderName);
    //calculateSpectrum(outFolderName);
    //calculateGradient(outFolderName);
    //calculateStatistics(outFolderName);
}

int main()
{
    //processCapillaries();
    processFocus();
    return 0;
}
