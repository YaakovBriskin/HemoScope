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
            SetMapInputFolder();
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
            SetMapInputFolder();
        }

        private void TxtOutputFolderMap_TextChanged(object sender, EventArgs e)
        {
            SetMapOutputFolder();
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
            SetMapOutputFolder();
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

        private void TxtInputFolderLock_TextChanged(object sender, EventArgs e)
        {
            SetLockInputFolder();
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
            SetLockInputFolder();
        }

        private void TxtOutputFolderLock_TextChanged(object sender, EventArgs e)
        {
            SetLockOutputFolder();
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
            SetLockOutputFolder();
        }

        private void BtnMode_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.overrideString(keyFocusingMethod, "Mode");
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnVariance_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.overrideString(keyFocusingMethod, "Variance");
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnSpectrum_Click(object sender, EventArgs e)
        {
            SetControlsEnabled(false);
            MapWrapper.loadPositionsZ();
            MapWrapper.overrideString(keyFocusingMethod, "Spectrum");
            MapWrapper.calculateDepth();
            SetControlsEnabled(true);
        }

        private void BtnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private const string keyInputMapFolder   = "HemoScope.Input.Map.Folder";
        private const string keyInputLockFolder  = "HemoScope.Input.Lock.Folder";
        private const string keyOutputMapFolder  = "HemoScope.Output.Map.Folder";
        private const string keyOutputLockFolder = "HemoScope.Output.Lock.Folder";
        private const string keyFocusingMethod   = "HemoScope.Procedures.Focusing.Method";

        private void SetMapInputFolder()
        {
            if (!string.IsNullOrWhiteSpace(TxtInputFolderMap.Text))
            {
                MapWrapper.overrideString(keyInputMapFolder, TxtInputFolderMap.Text);
            }
        }

        private void SetMapOutputFolder()
        {
            if (!string.IsNullOrWhiteSpace(TxtOutputFolderMap.Text))
            {
                MapWrapper.overrideString(keyOutputMapFolder, TxtOutputFolderMap.Text);
            }
        }

        private void SetLockInputFolder()
        {
            if (!string.IsNullOrWhiteSpace(TxtInputFolderLock.Text))
            {
                MapWrapper.overrideString(keyInputLockFolder, TxtInputFolderLock.Text);
            }
        }

        private void SetLockOutputFolder()
        {
            if (!string.IsNullOrWhiteSpace(TxtOutputFolderLock.Text))
            {
                MapWrapper.overrideString(keyOutputLockFolder, TxtOutputFolderLock.Text);
            }
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
