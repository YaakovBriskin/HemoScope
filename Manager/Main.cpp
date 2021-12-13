#include <iostream>
#include "Map3D.h"

void processCapillaries()
{
    buildMap();
    saveStiched();
    printValueAtTruncatedPos(400.0F, 200.0F, 24.6F); // example of map usage
    detectCapillaries();
    describeCapillaries();
    saveStiched();
}

void processFocus()
{
    loadPositionsZ();
    calculateDepth();
}

int main()
{
    loadConfig("../Config/Config.xml");
    initGeneralData();
    processCapillaries();
    //processFocus();
    return 0;
}
