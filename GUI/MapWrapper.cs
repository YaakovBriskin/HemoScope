using System.Runtime.InteropServices;

namespace GUI
{
    class MapWrapper
    {
        [DllImport(@"Map3D.dll")]
        public static extern void loadConfig(string configFilename);

        [DllImport(@"Map3D.dll")]
        public static extern void initGeneralData();

        [DllImport(@"Map3D.dll")]
        public static extern void buildMap();

        [DllImport(@"Map3D.dll")]
        public static extern void saveStiched();

        [DllImport(@"Map3D.dll")]
        public static extern void printValueAtTruncatedPos(float x, float y, float z);

        [DllImport(@"Map3D.dll")]
        public static extern void detectCapillaries();

        [DllImport(@"Map3D.dll")]

        public static extern void describeCapillaries();

        [DllImport(@"Map3D.dll")]
        public static extern void loadPositionsZ();

        [DllImport(@"Map3D.dll")]
        public static extern void buildSequence();

        [DllImport(@"Map3D.dll")]
        public static extern void saveProjections();

        [DllImport(@"Map3D.dll")]
        public static extern void calculateDepth();

        [DllImport(@"Map3D.dll")]
        public static extern void overrideInt(string key, int val);

        [DllImport(@"Map3D.dll")]
        public static extern void overrideFloat(string key, float val);

        [DllImport(@"Map3D.dll")]
        public static extern void overrideString(string key, string val);
    };
}
