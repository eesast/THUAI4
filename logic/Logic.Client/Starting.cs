using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Communication.Proto;

namespace Logic.Client
{
    public partial class Starting : Form
    {
        public Starting()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 0;
            comboBox3.SelectedIndex = 0;
            numericUpDown1.Value = 0;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Program.teamID = comboBox1.SelectedIndex+1;
            Program.jobType = (Communication.Proto.JobType)comboBox2.SelectedIndex;
            Program.playerID = comboBox3.SelectedIndex+1;
            Program.port = (ushort)numericUpDown1.Value;
            this.Close();
        }
    }
}
