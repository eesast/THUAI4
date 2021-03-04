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
            numericUpDown1.Value = 7777;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            teamid = comboBox1.SelectedIndex;
            job = (Communication.Proto.JobType)comboBox2.SelectedIndex;
            playerid = comboBox3.SelectedIndex;
            port = (ushort)numericUpDown1.Value;
            this.Close();
        }
        public Int64 teamid;
        public Int64 playerid;
        public JobType job;
        public ushort port;
    }
}