using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Incubator
{
    public partial class FormStatus : Form
    {
        public FormStatus()
        {
            InitializeComponent();
        }

        private void FormStatus_Load(object sender, EventArgs e)
        {

        }

        private MainForm MainForm => (MainForm)Owner;

        internal void OnDataReady(Dictionary<string, object> data)
        {
            lbCover.Text = data["HumidifierCover"].ToString();
            lbHeating.Text = data["Power"].ToString();
            lbHours.Text = data["HourOfIncubation"].ToString();
            lbTemperature.Text = data["CurrTemperature"].ToString();
            lbHumidity.Text = data["CurrentHumidity"].ToString();
            lbTray.Text = data["CurrentTrayPosition"].ToString();
        }
    }
}
