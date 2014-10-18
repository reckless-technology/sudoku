namespace Sudoku
{
    partial class MainForm
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.solveButton = new System.Windows.Forms.Button();
            this.clearButton = new System.Windows.Forms.Button();
            this.clueButton = new System.Windows.Forms.Button();
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.selectValueContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.selectClear = new System.Windows.Forms.ToolStripMenuItem();
            this.select1 = new System.Windows.Forms.ToolStripMenuItem();
            this.select2 = new System.Windows.Forms.ToolStripMenuItem();
            this.select3 = new System.Windows.Forms.ToolStripMenuItem();
            this.select4 = new System.Windows.Forms.ToolStripMenuItem();
            this.select5 = new System.Windows.Forms.ToolStripMenuItem();
            this.select6 = new System.Windows.Forms.ToolStripMenuItem();
            this.select7 = new System.Windows.Forms.ToolStripMenuItem();
            this.select8 = new System.Windows.Forms.ToolStripMenuItem();
            this.select9 = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenu = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveGameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openGameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            this.selectValueContextMenu.SuspendLayout();
            this.mainMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // solveButton
            // 
            this.solveButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.2F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.solveButton.Location = new System.Drawing.Point(330, 40);
            this.solveButton.Margin = new System.Windows.Forms.Padding(2);
            this.solveButton.Name = "solveButton";
            this.solveButton.Size = new System.Drawing.Size(75, 50);
            this.solveButton.TabIndex = 0;
            this.solveButton.Text = "Solve";
            this.solveButton.UseVisualStyleBackColor = true;
            this.solveButton.Click += new System.EventHandler(this.solveButton_Click);
            // 
            // clearButton
            // 
            this.clearButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.2F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.clearButton.Location = new System.Drawing.Point(330, 160);
            this.clearButton.Margin = new System.Windows.Forms.Padding(2);
            this.clearButton.Name = "clearButton";
            this.clearButton.Size = new System.Drawing.Size(75, 50);
            this.clearButton.TabIndex = 1;
            this.clearButton.Text = "Clear Board";
            this.clearButton.UseVisualStyleBackColor = true;
            this.clearButton.Click += new System.EventHandler(this.clearButton_Click);
            // 
            // clueButton
            // 
            this.clueButton.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.2F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.clueButton.Location = new System.Drawing.Point(330, 100);
            this.clueButton.Margin = new System.Windows.Forms.Padding(2);
            this.clueButton.Name = "clueButton";
            this.clueButton.Size = new System.Drawing.Size(75, 50);
            this.clueButton.TabIndex = 2;
            this.clueButton.Text = "Clue";
            this.clueButton.UseVisualStyleBackColor = true;
            this.clueButton.Click += new System.EventHandler(this.clueButton_Click);
            // 
            // pictureBox
            // 
            this.pictureBox.BackColor = System.Drawing.Color.White;
            this.pictureBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.pictureBox.Location = new System.Drawing.Point(11, 41);
            this.pictureBox.Margin = new System.Windows.Forms.Padding(2);
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.Size = new System.Drawing.Size(300, 300);
            this.pictureBox.TabIndex = 4;
            this.pictureBox.TabStop = false;
            this.pictureBox.MouseClick += new System.Windows.Forms.MouseEventHandler(this.gamePicBox_MouseClick);
            this.pictureBox.Paint += new System.Windows.Forms.PaintEventHandler(this.gamePicBox_Paint);
            // 
            // selectValueContextMenu
            // 
            this.selectValueContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.selectClear,
            this.select1,
            this.select2,
            this.select3,
            this.select4,
            this.select5,
            this.select6,
            this.select7,
            this.select8,
            this.select9});
            this.selectValueContextMenu.Name = "selectValueContextMenu";
            this.selectValueContextMenu.Size = new System.Drawing.Size(102, 224);
            this.selectValueContextMenu.Text = "Select value for square";
            // 
            // selectClear
            // 
            this.selectClear.Name = "selectClear";
            this.selectClear.Size = new System.Drawing.Size(101, 22);
            this.selectClear.Text = "Clear";
            this.selectClear.Click += new System.EventHandler(this.selectClear_Click);
            // 
            // select1
            // 
            this.select1.Name = "select1";
            this.select1.Size = new System.Drawing.Size(101, 22);
            this.select1.Text = "1";
            this.select1.Click += new System.EventHandler(this.select1_Click);
            // 
            // select2
            // 
            this.select2.Name = "select2";
            this.select2.Size = new System.Drawing.Size(101, 22);
            this.select2.Text = "2";
            this.select2.Click += new System.EventHandler(this.select2_Click);
            // 
            // select3
            // 
            this.select3.Name = "select3";
            this.select3.Size = new System.Drawing.Size(101, 22);
            this.select3.Text = "3";
            this.select3.Click += new System.EventHandler(this.select3_Click);
            // 
            // select4
            // 
            this.select4.Name = "select4";
            this.select4.Size = new System.Drawing.Size(101, 22);
            this.select4.Text = "4";
            this.select4.Click += new System.EventHandler(this.select4_Click);
            // 
            // select5
            // 
            this.select5.Name = "select5";
            this.select5.Size = new System.Drawing.Size(101, 22);
            this.select5.Text = "5";
            this.select5.Click += new System.EventHandler(this.select5_Click);
            // 
            // select6
            // 
            this.select6.Name = "select6";
            this.select6.Size = new System.Drawing.Size(101, 22);
            this.select6.Text = "6";
            this.select6.Click += new System.EventHandler(this.select6_Click);
            // 
            // select7
            // 
            this.select7.Name = "select7";
            this.select7.Size = new System.Drawing.Size(101, 22);
            this.select7.Text = "7";
            this.select7.Click += new System.EventHandler(this.select7_Click);
            // 
            // select8
            // 
            this.select8.Name = "select8";
            this.select8.Size = new System.Drawing.Size(101, 22);
            this.select8.Text = "8";
            this.select8.Click += new System.EventHandler(this.select8_Click);
            // 
            // select9
            // 
            this.select9.Name = "select9";
            this.select9.Size = new System.Drawing.Size(101, 22);
            this.select9.Text = "9";
            this.select9.Click += new System.EventHandler(this.select9_Click);
            // 
            // mainMenu
            // 
            this.mainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.mainMenu.Location = new System.Drawing.Point(0, 0);
            this.mainMenu.Name = "mainMenu";
            this.mainMenu.Padding = new System.Windows.Forms.Padding(4, 2, 0, 2);
            this.mainMenu.Size = new System.Drawing.Size(420, 24);
            this.mainMenu.TabIndex = 5;
            this.mainMenu.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveGameToolStripMenuItem,
            this.openGameToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // saveGameToolStripMenuItem
            // 
            this.saveGameToolStripMenuItem.Name = "saveGameToolStripMenuItem";
            this.saveGameToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.saveGameToolStripMenuItem.Text = "&Save game";
            this.saveGameToolStripMenuItem.Click += new System.EventHandler(this.saveGameToolStripMenuItem_Click);
            // 
            // openGameToolStripMenuItem
            // 
            this.openGameToolStripMenuItem.Name = "openGameToolStripMenuItem";
            this.openGameToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.openGameToolStripMenuItem.Text = "&Open game";
            this.openGameToolStripMenuItem.Click += new System.EventHandler(this.openGameToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.DefaultExt = "sud";
            this.saveFileDialog.Filter = "Sudoku files (*.sud)|*.sud|All files (*.*)|*.*";
            this.saveFileDialog.Title = "Select the file to save";
            this.saveFileDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.saveFileDialog_FileOk);
            // 
            // openFileDialog
            // 
            this.openFileDialog.DefaultExt = "sud";
            this.openFileDialog.Filter = "Sudoku files (*.sud)|*.sud|All files (*.*)|*.*";
            this.openFileDialog.Title = "Select the file to open";
            this.openFileDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog_FileOk);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(420, 370);
            this.Controls.Add(this.mainMenu);
            this.Controls.Add(this.pictureBox);
            this.Controls.Add(this.solveButton);
            this.Controls.Add(this.clearButton);
            this.Controls.Add(this.clueButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MainMenuStrip = this.mainMenu;
            this.Margin = new System.Windows.Forms.Padding(2);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.Text = "Jonny\'s Sudoku Solver";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            this.selectValueContextMenu.ResumeLayout(false);
            this.mainMenu.ResumeLayout(false);
            this.mainMenu.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button solveButton;
        private System.Windows.Forms.Button clearButton;
        private System.Windows.Forms.Button clueButton;
        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.ContextMenuStrip selectValueContextMenu;
        private System.Windows.Forms.ToolStripMenuItem select1;
        private System.Windows.Forms.ToolStripMenuItem select2;
        private System.Windows.Forms.ToolStripMenuItem select3;
        private System.Windows.Forms.ToolStripMenuItem select4;
        private System.Windows.Forms.ToolStripMenuItem select5;
        private System.Windows.Forms.ToolStripMenuItem select6;
        private System.Windows.Forms.ToolStripMenuItem select7;
        private System.Windows.Forms.ToolStripMenuItem select8;
        private System.Windows.Forms.ToolStripMenuItem select9;
        private System.Windows.Forms.ToolStripMenuItem selectClear;
        private System.Windows.Forms.MenuStrip mainMenu;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveGameToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openGameToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
    }
}

