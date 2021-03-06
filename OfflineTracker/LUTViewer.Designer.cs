﻿namespace OfflineTracker
{
	partial class LUTViewer
	{
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.trackBarBeadIndex = new System.Windows.Forms.TrackBar();
            this.textBeadIndex = new System.Windows.Forms.TextBox();
            this.pictureBoxLUT = new System.Windows.Forms.PictureBox();
            this.pictureBoxBeadImg = new System.Windows.Forms.PictureBox();
            this.labelInfo = new System.Windows.Forms.Label();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarBeadIndex)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLUT)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxBeadImg)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.SuspendLayout();
            // 
            // trackBarBeadIndex
            // 
            this.trackBarBeadIndex.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.trackBarBeadIndex.Location = new System.Drawing.Point(4, 197);
            this.trackBarBeadIndex.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.trackBarBeadIndex.Name = "trackBarBeadIndex";
            this.trackBarBeadIndex.Size = new System.Drawing.Size(227, 45);
            this.trackBarBeadIndex.TabIndex = 0;
            this.trackBarBeadIndex.Scroll += new System.EventHandler(this.trackBarBeadIndex_Scroll);
            // 
            // textBeadIndex
            // 
            this.textBeadIndex.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.textBeadIndex.Location = new System.Drawing.Point(232, 197);
            this.textBeadIndex.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.textBeadIndex.Name = "textBeadIndex";
            this.textBeadIndex.Size = new System.Drawing.Size(38, 20);
            this.textBeadIndex.TabIndex = 1;
            // 
            // pictureBoxLUT
            // 
            this.pictureBoxLUT.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBoxLUT.Location = new System.Drawing.Point(0, 0);
            this.pictureBoxLUT.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.pictureBoxLUT.Name = "pictureBoxLUT";
            this.pictureBoxLUT.Size = new System.Drawing.Size(125, 176);
            this.pictureBoxLUT.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBoxLUT.TabIndex = 2;
            this.pictureBoxLUT.TabStop = false;
            // 
            // pictureBoxBeadImg
            // 
            this.pictureBoxBeadImg.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBoxBeadImg.Location = new System.Drawing.Point(0, 0);
            this.pictureBoxBeadImg.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.pictureBoxBeadImg.Name = "pictureBoxBeadImg";
            this.pictureBoxBeadImg.Size = new System.Drawing.Size(140, 176);
            this.pictureBoxBeadImg.TabIndex = 3;
            this.pictureBoxBeadImg.TabStop = false;
            // 
            // labelInfo
            // 
            this.labelInfo.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.labelInfo.AutoSize = true;
            this.labelInfo.Location = new System.Drawing.Point(11, 180);
            this.labelInfo.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.labelInfo.Name = "labelInfo";
            this.labelInfo.Size = new System.Drawing.Size(35, 13);
            this.labelInfo.TabIndex = 4;
            this.labelInfo.Text = "label1";
            // 
            // splitContainer1
            // 
            this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer1.Location = new System.Drawing.Point(2, 2);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.pictureBoxLUT);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.pictureBoxBeadImg);
            this.splitContainer1.Size = new System.Drawing.Size(268, 176);
            this.splitContainer1.SplitterDistance = 125;
            this.splitContainer1.SplitterWidth = 3;
            this.splitContainer1.TabIndex = 5;
            // 
            // LUTViewer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.labelInfo);
            this.Controls.Add(this.textBeadIndex);
            this.Controls.Add(this.trackBarBeadIndex);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "LUTViewer";
            this.Size = new System.Drawing.Size(271, 228);
            ((System.ComponentModel.ISupportInitialize)(this.trackBarBeadIndex)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxLUT)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxBeadImg)).EndInit();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.TrackBar trackBarBeadIndex;
		private System.Windows.Forms.TextBox textBeadIndex;
		private System.Windows.Forms.PictureBox pictureBoxLUT;
		private System.Windows.Forms.PictureBox pictureBoxBeadImg;
		private System.Windows.Forms.Label labelInfo;
		private System.Windows.Forms.SplitContainer splitContainer1;
	}
}
