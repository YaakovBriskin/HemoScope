
namespace GUI
{
    partial class FormMain
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.BoxMap = new System.Windows.Forms.GroupBox();
            this.BtnBrowseMapOutputFolder = new System.Windows.Forms.Button();
            this.TxtOutputFolderMap = new System.Windows.Forms.TextBox();
            this.LblOutputFolderMap = new System.Windows.Forms.Label();
            this.BtnBrowseMapInputFolder = new System.Windows.Forms.Button();
            this.TxtInputFolderMap = new System.Windows.Forms.TextBox();
            this.LblInputFolderMap = new System.Windows.Forms.Label();
            this.BtnDetect = new System.Windows.Forms.Button();
            this.BtnDescribe = new System.Windows.Forms.Button();
            this.BtnBuildMap = new System.Windows.Forms.Button();
            this.BoxLock = new System.Windows.Forms.GroupBox();
            this.BtnSpectrum = new System.Windows.Forms.Button();
            this.BtnVariance = new System.Windows.Forms.Button();
            this.BtnMode = new System.Windows.Forms.Button();
            this.BtnBrowseLockOutputFolder = new System.Windows.Forms.Button();
            this.BtnBrowseLockInputFolder = new System.Windows.Forms.Button();
            this.TxtOutputFolderLock = new System.Windows.Forms.TextBox();
            this.TxtInputFolderLock = new System.Windows.Forms.TextBox();
            this.LblOutputFolderLock = new System.Windows.Forms.Label();
            this.LblInputFolderLock = new System.Windows.Forms.Label();
            this.BtnClose = new System.Windows.Forms.Button();
            this.BoxMap.SuspendLayout();
            this.BoxLock.SuspendLayout();
            this.SuspendLayout();
            // 
            // BoxMap
            // 
            this.BoxMap.Controls.Add(this.BtnBrowseMapOutputFolder);
            this.BoxMap.Controls.Add(this.TxtOutputFolderMap);
            this.BoxMap.Controls.Add(this.LblOutputFolderMap);
            this.BoxMap.Controls.Add(this.BtnBrowseMapInputFolder);
            this.BoxMap.Controls.Add(this.TxtInputFolderMap);
            this.BoxMap.Controls.Add(this.LblInputFolderMap);
            this.BoxMap.Controls.Add(this.BtnDetect);
            this.BoxMap.Controls.Add(this.BtnDescribe);
            this.BoxMap.Controls.Add(this.BtnBuildMap);
            this.BoxMap.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BoxMap.Location = new System.Drawing.Point(30, 20);
            this.BoxMap.Name = "BoxMap";
            this.BoxMap.Size = new System.Drawing.Size(800, 240);
            this.BoxMap.TabIndex = 5;
            this.BoxMap.TabStop = false;
            this.BoxMap.Text = "Capillaries 3D map";
            // 
            // BtnBrowseMapOutputFolder
            // 
            this.BtnBrowseMapOutputFolder.Location = new System.Drawing.Point(690, 124);
            this.BtnBrowseMapOutputFolder.Name = "BtnBrowseMapOutputFolder";
            this.BtnBrowseMapOutputFolder.Size = new System.Drawing.Size(80, 30);
            this.BtnBrowseMapOutputFolder.TabIndex = 12;
            this.BtnBrowseMapOutputFolder.Text = "Browse";
            this.BtnBrowseMapOutputFolder.UseVisualStyleBackColor = true;
            this.BtnBrowseMapOutputFolder.Click += new System.EventHandler(this.BtnBrowseMapOutputFolder_Click);
            // 
            // TxtOutputFolderMap
            // 
            this.TxtOutputFolderMap.Font = new System.Drawing.Font("Segoe UI", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.TxtOutputFolderMap.Location = new System.Drawing.Point(30, 127);
            this.TxtOutputFolderMap.Name = "TxtOutputFolderMap";
            this.TxtOutputFolderMap.Size = new System.Drawing.Size(640, 25);
            this.TxtOutputFolderMap.TabIndex = 11;
            // 
            // LblOutputFolderMap
            // 
            this.LblOutputFolderMap.AutoSize = true;
            this.LblOutputFolderMap.Location = new System.Drawing.Point(30, 100);
            this.LblOutputFolderMap.Name = "LblOutputFolderMap";
            this.LblOutputFolderMap.Size = new System.Drawing.Size(324, 21);
            this.LblOutputFolderMap.TabIndex = 10;
            this.LblOutputFolderMap.Text = "Output folder of capillaries processing results";
            // 
            // BtnBrowseMapInputFolder
            // 
            this.BtnBrowseMapInputFolder.Location = new System.Drawing.Point(690, 54);
            this.BtnBrowseMapInputFolder.Name = "BtnBrowseMapInputFolder";
            this.BtnBrowseMapInputFolder.Size = new System.Drawing.Size(80, 30);
            this.BtnBrowseMapInputFolder.TabIndex = 9;
            this.BtnBrowseMapInputFolder.Text = "Browse";
            this.BtnBrowseMapInputFolder.UseVisualStyleBackColor = true;
            this.BtnBrowseMapInputFolder.Click += new System.EventHandler(this.BtnBrowseMapInputFolder_Click);
            // 
            // TxtInputFolderMap
            // 
            this.TxtInputFolderMap.Font = new System.Drawing.Font("Segoe UI", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.TxtInputFolderMap.Location = new System.Drawing.Point(30, 57);
            this.TxtInputFolderMap.Name = "TxtInputFolderMap";
            this.TxtInputFolderMap.Size = new System.Drawing.Size(640, 25);
            this.TxtInputFolderMap.TabIndex = 8;
            // 
            // LblInputFolderMap
            // 
            this.LblInputFolderMap.AutoSize = true;
            this.LblInputFolderMap.Location = new System.Drawing.Point(30, 30);
            this.LblInputFolderMap.Name = "LblInputFolderMap";
            this.LblInputFolderMap.Size = new System.Drawing.Size(221, 21);
            this.LblInputFolderMap.TabIndex = 7;
            this.LblInputFolderMap.Text = "Input folder of stitched images";
            // 
            // BtnDetect
            // 
            this.BtnDetect.BackColor = System.Drawing.Color.LightSkyBlue;
            this.BtnDetect.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BtnDetect.Location = new System.Drawing.Point(270, 180);
            this.BtnDetect.Name = "BtnDetect";
            this.BtnDetect.Size = new System.Drawing.Size(160, 40);
            this.BtnDetect.TabIndex = 6;
            this.BtnDetect.Text = "Detect capillaries";
            this.BtnDetect.UseVisualStyleBackColor = false;
            this.BtnDetect.Click += new System.EventHandler(this.BtnDetect_Click);
            // 
            // BtnDescribe
            // 
            this.BtnDescribe.BackColor = System.Drawing.Color.LightSkyBlue;
            this.BtnDescribe.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BtnDescribe.Location = new System.Drawing.Point(510, 180);
            this.BtnDescribe.Name = "BtnDescribe";
            this.BtnDescribe.Size = new System.Drawing.Size(160, 40);
            this.BtnDescribe.TabIndex = 5;
            this.BtnDescribe.Text = "Describe capillaries";
            this.BtnDescribe.UseVisualStyleBackColor = false;
            this.BtnDescribe.Click += new System.EventHandler(this.BtnDescribe_Click);
            // 
            // BtnBuildMap
            // 
            this.BtnBuildMap.BackColor = System.Drawing.Color.LightSkyBlue;
            this.BtnBuildMap.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BtnBuildMap.Location = new System.Drawing.Point(30, 180);
            this.BtnBuildMap.Name = "BtnBuildMap";
            this.BtnBuildMap.Size = new System.Drawing.Size(160, 40);
            this.BtnBuildMap.TabIndex = 4;
            this.BtnBuildMap.Text = "Build 3D map";
            this.BtnBuildMap.UseVisualStyleBackColor = false;
            this.BtnBuildMap.Click += new System.EventHandler(this.BtnBuildMap_Click);
            // 
            // BoxLock
            // 
            this.BoxLock.Controls.Add(this.BtnSpectrum);
            this.BoxLock.Controls.Add(this.BtnVariance);
            this.BoxLock.Controls.Add(this.BtnMode);
            this.BoxLock.Controls.Add(this.BtnBrowseLockOutputFolder);
            this.BoxLock.Controls.Add(this.BtnBrowseLockInputFolder);
            this.BoxLock.Controls.Add(this.TxtOutputFolderLock);
            this.BoxLock.Controls.Add(this.TxtInputFolderLock);
            this.BoxLock.Controls.Add(this.LblOutputFolderLock);
            this.BoxLock.Controls.Add(this.LblInputFolderLock);
            this.BoxLock.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BoxLock.Location = new System.Drawing.Point(30, 280);
            this.BoxLock.Name = "BoxLock";
            this.BoxLock.Size = new System.Drawing.Size(800, 240);
            this.BoxLock.TabIndex = 6;
            this.BoxLock.TabStop = false;
            this.BoxLock.Text = "Lock Z position";
            // 
            // BtnSpectrum
            // 
            this.BtnSpectrum.BackColor = System.Drawing.Color.PaleGreen;
            this.BtnSpectrum.Location = new System.Drawing.Point(510, 180);
            this.BtnSpectrum.Name = "BtnSpectrum";
            this.BtnSpectrum.Size = new System.Drawing.Size(160, 40);
            this.BtnSpectrum.TabIndex = 8;
            this.BtnSpectrum.Text = "Spectral analysis";
            this.BtnSpectrum.UseVisualStyleBackColor = false;
            this.BtnSpectrum.Click += new System.EventHandler(this.BtnSpectrum_Click);
            // 
            // BtnVariance
            // 
            this.BtnVariance.BackColor = System.Drawing.Color.PaleGreen;
            this.BtnVariance.Location = new System.Drawing.Point(270, 180);
            this.BtnVariance.Name = "BtnVariance";
            this.BtnVariance.Size = new System.Drawing.Size(160, 40);
            this.BtnVariance.TabIndex = 7;
            this.BtnVariance.Text = "Histogram variance";
            this.BtnVariance.UseVisualStyleBackColor = false;
            this.BtnVariance.Click += new System.EventHandler(this.BtnVariance_Click);
            // 
            // BtnMode
            // 
            this.BtnMode.BackColor = System.Drawing.Color.PaleGreen;
            this.BtnMode.Location = new System.Drawing.Point(30, 180);
            this.BtnMode.Name = "BtnMode";
            this.BtnMode.Size = new System.Drawing.Size(160, 40);
            this.BtnMode.TabIndex = 6;
            this.BtnMode.Text = "Histogram mode";
            this.BtnMode.UseVisualStyleBackColor = false;
            this.BtnMode.Click += new System.EventHandler(this.BtnMode_Click);
            // 
            // BtnBrowseLockOutputFolder
            // 
            this.BtnBrowseLockOutputFolder.Location = new System.Drawing.Point(690, 124);
            this.BtnBrowseLockOutputFolder.Name = "BtnBrowseLockOutputFolder";
            this.BtnBrowseLockOutputFolder.Size = new System.Drawing.Size(80, 30);
            this.BtnBrowseLockOutputFolder.TabIndex = 5;
            this.BtnBrowseLockOutputFolder.Text = "Browse";
            this.BtnBrowseLockOutputFolder.UseVisualStyleBackColor = true;
            this.BtnBrowseLockOutputFolder.Click += new System.EventHandler(this.BtnBrowseLockOutputFolder_Click);
            // 
            // BtnBrowseLockInputFolder
            // 
            this.BtnBrowseLockInputFolder.Location = new System.Drawing.Point(690, 54);
            this.BtnBrowseLockInputFolder.Name = "BtnBrowseLockInputFolder";
            this.BtnBrowseLockInputFolder.Size = new System.Drawing.Size(80, 30);
            this.BtnBrowseLockInputFolder.TabIndex = 4;
            this.BtnBrowseLockInputFolder.Text = "Browse";
            this.BtnBrowseLockInputFolder.UseVisualStyleBackColor = true;
            this.BtnBrowseLockInputFolder.Click += new System.EventHandler(this.BtnBrowseLockInputFolder_Click);
            // 
            // TxtOutputFolderLock
            // 
            this.TxtOutputFolderLock.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.TxtOutputFolderLock.Location = new System.Drawing.Point(30, 127);
            this.TxtOutputFolderLock.Name = "TxtOutputFolderLock";
            this.TxtOutputFolderLock.Size = new System.Drawing.Size(640, 25);
            this.TxtOutputFolderLock.TabIndex = 3;
            // 
            // TxtInputFolderLock
            // 
            this.TxtInputFolderLock.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.TxtInputFolderLock.Location = new System.Drawing.Point(30, 57);
            this.TxtInputFolderLock.Name = "TxtInputFolderLock";
            this.TxtInputFolderLock.Size = new System.Drawing.Size(640, 25);
            this.TxtInputFolderLock.TabIndex = 2;
            // 
            // LblOutputFolderLock
            // 
            this.LblOutputFolderLock.AutoSize = true;
            this.LblOutputFolderLock.Location = new System.Drawing.Point(30, 100);
            this.LblOutputFolderLock.Name = "LblOutputFolderLock";
            this.LblOutputFolderLock.Size = new System.Drawing.Size(282, 21);
            this.LblOutputFolderLock.TabIndex = 1;
            this.LblOutputFolderLock.Text = "Output folder of image focusing results";
            // 
            // LblInputFolderLock
            // 
            this.LblInputFolderLock.AutoSize = true;
            this.LblInputFolderLock.Location = new System.Drawing.Point(30, 30);
            this.LblInputFolderLock.Name = "LblInputFolderLock";
            this.LblInputFolderLock.Size = new System.Drawing.Size(259, 21);
            this.LblInputFolderLock.TabIndex = 7;
            this.LblInputFolderLock.Text = "Input folder of wide and line images";
            // 
            // BtnClose
            // 
            this.BtnClose.Font = new System.Drawing.Font("Segoe UI", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point);
            this.BtnClose.Location = new System.Drawing.Point(720, 550);
            this.BtnClose.Name = "BtnClose";
            this.BtnClose.Size = new System.Drawing.Size(110, 40);
            this.BtnClose.TabIndex = 0;
            this.BtnClose.Text = "Close";
            this.BtnClose.UseVisualStyleBackColor = true;
            this.BtnClose.Click += new System.EventHandler(this.BtnClose_Click);
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(864, 611);
            this.Controls.Add(this.BtnClose);
            this.Controls.Add(this.BoxLock);
            this.Controls.Add(this.BoxMap);
            this.Name = "FormMain";
            this.Text = "HemoScope";
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.BoxMap.ResumeLayout(false);
            this.BoxMap.PerformLayout();
            this.BoxLock.ResumeLayout(false);
            this.BoxLock.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.GroupBox BoxMap;
        private System.Windows.Forms.Button BtnDetect;
        private System.Windows.Forms.Button BtnDescribe;
        private System.Windows.Forms.Button BtnBuildMap;
        private System.Windows.Forms.Label LblInputFolderMap;
        private System.Windows.Forms.TextBox TxtInputFolderMap;
        private System.Windows.Forms.Button BtnBrowseMapInputFolder;
        private System.Windows.Forms.Label LblOutputFolderMap;
        private System.Windows.Forms.GroupBox BoxLock;
        private System.Windows.Forms.TextBox TxtOutputFolderMap;
        private System.Windows.Forms.Button BtnBrowseMapOutputFolder;
        private System.Windows.Forms.Button BtnClose;
        private System.Windows.Forms.Label LblInputFolderLock;
        private System.Windows.Forms.Label LblOutputFolderLock;
        private System.Windows.Forms.TextBox TxtOutputFolderLock;
        private System.Windows.Forms.TextBox TxtInputFolderLock;
        private System.Windows.Forms.Button BtnBrowseLockOutputFolder;
        private System.Windows.Forms.Button BtnBrowseLockInputFolder;
        private System.Windows.Forms.Button BtnSpectrum;
        private System.Windows.Forms.Button BtnVariance;
        private System.Windows.Forms.Button BtnMode;
    }
}

