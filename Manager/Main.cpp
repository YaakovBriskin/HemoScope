#include <iostream>
#include "Map3D.h"

int main()
{
    const std::string folderName = "17";
    buildMap("../XYZ/" + folderName);
    saveStiched("../Output/" + folderName, false);
    printValueAtTruncatedPos(400.0F, 200.0F, 24.6F);
    detectCapillaries("../Output/" + folderName);
    selectBestLayer();
    describeCapillaries("../Output/" + folderName);
    saveStiched("../Output/" + folderName, true);
    return 0;
}
