using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace Sudoku
{
    public partial class MainForm : Form
    {
        private Pen thickPen;
        private Pen thinPen;
        private Font arialFont;
        private int xMouse, yMouse;
        private StringFormat format;
        private ISudokuSolver solver;

        public MainForm()
        {
            thickPen = new Pen(Color.Black, 5.0f);
            thinPen = new Pen(Color.Black, 1.0f);
            arialFont = new Font("Arial", 20.0f);
            format = new StringFormat();
            format.Alignment = StringAlignment.Center;
            format.LineAlignment = StringAlignment.Center;
            solver = new Solver(3) as ISudokuSolver;
            InitializeComponent();
        }

        private void gamePicBox_Paint(object sender, PaintEventArgs e)
        {
            Graphics graphics = e.Graphics;
            DrawGrid(graphics);
            Point point;
            for (int x = 0; x != 9; x++)
                for (int y = 0; y != 9; y++)
                {
                    if (solver[x, y] != 0)
                    {
                        point = new Point(x * (pictureBox.Width / 9) + pictureBox.Width / 18,
                                            y * (pictureBox.Height / 9) + pictureBox.Width / 18 + 2);
                        graphics.DrawString(solver[x, y].ToString(), arialFont, Brushes.Black, point, format);
                    }
                }
        }

        private void DrawGrid(Graphics graphics)
        {
            for (int y = 0; y <= pictureBox.Height; y += (pictureBox.Height / 3))
            {
                graphics.DrawLine(thickPen, 0, y, pictureBox.Width, y);
            }
            for (int x = 0; x <= pictureBox.Width; x += (pictureBox.Width / 3))
            {
                graphics.DrawLine(thickPen, x, 0, x, pictureBox.Height);
            }
            for (int y = 0; y <= pictureBox.Height; y += (pictureBox.Height / 9))
            {
                graphics.DrawLine(thinPen, 0, y, pictureBox.Width, y);
            }
            for (int x = 0; x <= pictureBox.Width; x += (pictureBox.Width / 9))
            {
                graphics.DrawLine(thinPen, x, 0, x, pictureBox.Height);
            }
        }

        private void gamePicBox_MouseClick(object sender, MouseEventArgs e)
        {
            this.xMouse = e.X * 9 / pictureBox.Width;
            this.yMouse = e.Y * 9 / pictureBox.Height;
            selectValueContextMenu.Show(this.pictureBox, e.X, e.Y);
        }

        private void select1_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 1;
            pictureBox.Invalidate();
        }

        private void select2_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 2;
            pictureBox.Invalidate();
        }

        private void select3_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 3;
            pictureBox.Invalidate();
        }

        private void select4_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 4;
            pictureBox.Invalidate();
        }

        private void select5_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 5;
            pictureBox.Invalidate();
        }

        private void select6_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 6;
            pictureBox.Invalidate();
        }

        private void select7_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 7;
            pictureBox.Invalidate();
        }

        private void select8_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 8;
            pictureBox.Invalidate();
        }

        private void select9_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 9;
            pictureBox.Invalidate();
        }

        private void selectClear_Click(object sender, EventArgs e)
        {
            solver[xMouse, yMouse] = 0;
            pictureBox.Invalidate();
        }

        private void clearButton_Click(object sender, EventArgs e)
        {
            solver = new Solver(3) as ISudokuSolver;
            pictureBox.Invalidate();
        }

        private void solveButton_Click(object sender, EventArgs e)
        {
            solver.Solve();
            if (solver.hasBeenSolved != true)
                MessageBox.Show("Unable to find solution.");
            else
                MessageBox.Show("Sudoku successfully solved!");
            pictureBox.Invalidate();
        }

        private void saveGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            saveFileDialog.ShowDialog();
        }

        private void saveFileDialog_FileOk(object sender, CancelEventArgs e)
        {
            byte[] fileBytes = new byte[81];
            for (int x = 0; x != 9; x++)
            {
                for (int y = 0; y != 9; y++)
                {
                    fileBytes[x + 9 * y] = (byte)solver[x, y];
                }
            }
            try
            {
                System.IO.FileStream fileWriter =
                    new System.IO.FileStream(saveFileDialog.FileName, System.IO.FileMode.OpenOrCreate);
                fileWriter.Write(fileBytes, 0, fileBytes.Length);
                fileWriter.Close();
                fileWriter.Dispose();
            }
            catch (System.Exception)
            {
                MessageBox.Show("It was not possible to save the game.", "Error");
            }

        }

        private void openGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            openFileDialog.ShowDialog();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void openFileDialog_FileOk(object sender, CancelEventArgs e)
        {
            try
            {
                solver = new Solver(3) as ISudokuSolver;
                System.IO.FileStream fileReader =
                    new System.IO.FileStream(openFileDialog.FileName, System.IO.FileMode.Open, System.IO.FileAccess.Read,
                    System.IO.FileShare.Read, 81, false);
                byte[] fileBytes = new byte[81];
                fileReader.Read(fileBytes, 0, fileBytes.Length);
                fileReader.Close();
                fileReader.Dispose();
                foreach (byte b in fileBytes)
                {
                    if (b > 9)
                        throw new System.Exception("Incorrect file format");
                }
                for (int x = 0; x != 9; x++)
                {
                    for (int y = 0; y != 9; y++)
                    {
                        solver[x, y] = fileBytes[x + 9 * y];
                    }
                }
                pictureBox.Invalidate();
            }
            catch (System.Exception)
            {
                MessageBox.Show("The file does not contain a valid Sudoku board", "Error");
                solver = new Solver(3) as ISudokuSolver;
                pictureBox.Invalidate();
            }
        }

        private void clueButton_Click(object sender, EventArgs e)
        {
            Move move = new Move();
            bool hasGotMove = solver.GetNextMove(ref move);
            if (!hasGotMove)
            {
                MessageBox.Show("Unable to find move");
            }
            pictureBox.Invalidate();
        }
    }
}