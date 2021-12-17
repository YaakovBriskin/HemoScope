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

        private void BtnBrowseMapInputFolder_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();
                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    TxtInputFolderMap.Text = fbd.SelectedPath;
                }
            }
        }

        private void BtnBrowseMapOutputFolder_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();
                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    TxtOutputFolderMap.Text = fbd.SelectedPath;
                }
            }
        }

        private void BtnBuildMap_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            SetControlsEnabled(true);
        }

        private void BtnDetect_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            MapWrapper.detectCapillaries();
            MapWrapper.saveStiched();
            SetControlsEnabled(true);
        }

        private void BtnDescribe_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.buildMap();
            MapWrapper.saveStiched();
            MapWrapper.detectCapillaries();
            MapWrapper.describeCapillaries();
            MapWrapper.saveStiched();
            SetControlsEnabled(true);
        }

        private void BtnBrowseLockInputFolder_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();
                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    TxtInputFolderLock.Text = fbd.SelectedPath;
                }
            }
        }

        private void BtnBrowseLockOutputFolder_Click(object sender, EventArgs e)
        {
            using (FolderBrowserDialog fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();
                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    TxtOutputFolderLock.Text = fbd.SelectedPath;
                }
            }
        }

        private void BtnMode_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnVariance_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnSpectrum_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void SetControlsEnabled(bool state)
        {
            SetControlsInGroupBoxEnabled(BoxMap, state);
            SetControlsInGroupBoxEnabled(BoxLock, state);
            BtnClose.Enabled = state;
        }

        private void SetControlsInGroupBoxEnabled(GroupBox groupBox, bool state)
        {
            foreach (Control control in groupBox.Controls)
            {
                Type controlType = control.GetType();
                if ((controlType == typeof(TextBox)) || (controlType == typeof(Button)))
                {
                    control.Enabled = state;
                }
            }
        }
    }
}
