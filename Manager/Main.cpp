#include <iostream>
#include "Map3D.h"

int main()
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

    return 0;
}
