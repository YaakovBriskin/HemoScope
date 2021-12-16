using System;
using System.Windows.Forms;

namespace GUI
{
    public partial class FormMain : Form
    {
        public FormMain()
        {
            InitializeComponent();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            MapWrapper.loadConfig(@"..\..\..\Config\Config.xml");
            MapWrapper.initGeneralData();
        }

        private void TxtInputFolderMap_TextChanged(object sender, EventArgs e)
        {
            EnableMapButtons();
        }

        private void TxtOutputFolderMap_TextChanged(object sender, EventArgs e)
        {
            EnableMapButtons();
        }

        private void BtnBrowseMapInputFolder_Click(object sender, EventArgs e)
        {
            // OPEN FOLDER BROWSER
        }

        private void BtnBrowseMapOutputFolder_Click(object sender, EventArgs e)
        {
            // OPEN FOLDER BROWSER
        }

        private void BtnBuildMap_Click(object sender, EventArgs e)
        {
            EnableMapButtons();
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            DisableMapButtons();
        }

        private void BtnDetect_Click(object sender, EventArgs e)
        {
            EnableMapButtons();
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            MapWrapper.detectCapillaries();
            MapWrapper.saveStiched();
            DisableMapButtons();
        }

        private void BtnDescribe_Click(object sender, EventArgs e)
        {
            EnableMapButtons();
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            MapWrapper.detectCapillaries();
            MapWrapper.describeCapillaries();
            MapWrapper.saveStiched();
            DisableMapButtons();
        }

        private void BtnBrowseLockInputFolder_Click(object sender, EventArgs e)
        {
            // OPEN FOLDER BROWSER
        }

        private void BtnBrowseLockOutputFolder_Click(object sender, EventArgs e)
        {
            // OPEN FOLDER BROWSER
        }

        private void BtnMode_Click(object sender, EventArgs e)
        {

        }

        private void BtnVariance_Click(object sender, EventArgs e)
        {

        }

        private void BtnSpectrum_Click(object sender, EventArgs e)
        {

        }

        private void BtnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void DisableMapButtons()
        {
            BtnBrowseMapInputFolder.Enabled = false;
            BtnBrowseMapOutputFolder.Enabled = false;
            BtnBuildMap.Enabled = false;
            BtnDetect.Enabled = false;
            BtnDescribe.Enabled = false;
        }

        private void EnableMapButtons()
        {
            BtnBrowseMapInputFolder.Enabled = false;
            BtnBrowseMapOutputFolder.Enabled = false;
            BtnBuildMap.Enabled = false;
            BtnDetect.Enabled = false;
            BtnDescribe.Enabled = false;
        }
    }
}
