using Newtonsoft.Json;
using SimpleHttp;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

namespace Incubator
{
    public partial class MainForm : Form
    {
        private readonly SerialPort _serialPort;
        private Tlv _tlv;
        private string _incubatorData = "{}";
        private readonly Dictionary<string, object> _incubatorDataDict = new Dictionary<string, object>();
        private FormStatus _formStatus;

        internal Dictionary<string, object> Data
        {
            get
            {
                lock (_incubatorDataDict)
                {
                    var res = new Dictionary<string, object>();
                    foreach (var kvp in _incubatorDataDict)
                    {
                        res.Add(kvp.Key, kvp.Value);
                    }

                    return res;
                }
            }
        }

        public MainForm()
        {
            InitializeComponent();

            _serialPort = new SerialPort(tbComPort.Text.Trim(), 9600, Parity.None);
            _serialPort.DtrEnable = false;
            _serialPort.DataReceived += _serialPort_DataReceived;
            _serialPort.ReadTimeout = 0;
        }

        private void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (_tlv == null)
            {
                _tlv = new Tlv();
            }

            var bytesToRead = _serialPort.BytesToRead;

            while (bytesToRead-- > 0)
            {
                if (_tlv.ReadByte(_serialPort.ReadByte()))
                {
                    var tlv = _tlv;
                    _tlv = new Tlv();
                    Invoke((MethodInvoker)delegate
                    {
                        // Running on the UI thread
                        OnTlvReceived(tlv);
                    });
                }
            }
        }

        private void OnTlvReceived(Tlv tlv)
        {
            var unixTime = (long)(DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds;
            var dict = new Dictionary<string, object>();
            dict["CurrTemperature"] = tlv.GetInt16Value() / 10.0f;
            dict["TargetTemperature"] = tlv.GetInt16Value() / 10.0f;
            dict["CurrentHumidity"] = tlv.GetInt16Value();
            dict["TargetHumidity"] = tlv.GetInt16Value();
            dict["CurrentTrayPosition"] = (char)tlv.GetInt8Value();
            dict["Power"] = tlv.GetInt16Value();
            dict["HumidifierCover"] = (char)tlv.GetInt8Value();
            dict["Error"] = (char)tlv.GetInt8Value();
            dict["Paused"] = (char)tlv.GetInt8Value();
            dict["HourOfIncubation"] = tlv.GetInt16Value();
            dict["Unixtime"] = unixTime;
            dict["Time"] = DateTime.Now.ToString(CultureInfo.InvariantCulture);
            lock (_incubatorDataDict)
            {
                _incubatorDataDict.Clear();
                foreach (var kvp in dict)
                {
                    _incubatorDataDict.Add(kvp.Key, kvp.Value);
                }
            }

            _formStatus.OnDataReady(Data);
            _incubatorData = JsonConvert.SerializeObject(dict, Formatting.Indented);
        }



        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (_serialPort.IsOpen)
            {
                _serialPort.Close();
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (!_serialPort.IsOpen) return;
            var tlvAskState = new Tlv();
            tlvAskState.Type = TlvType.GetCurrentState;
            tlvAskState.Write(_serialPort.BaseStream);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var tlvAskState = new Tlv();
            tlvAskState.Type = TlvType.Pause;
            tlvAskState.Write(_serialPort.BaseStream);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            var tlvAskState = new Tlv();
            tlvAskState.Type = TlvType.Start;
            tlvAskState.Write(_serialPort.BaseStream);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            var tlvAskState = new Tlv();
            tlvAskState.Type = TlvType.Continue;
            tlvAskState.Write(_serialPort.BaseStream);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            if (!_serialPort.IsOpen)
            {
                _serialPort.Open();
            }
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            Console.WriteLine("START");
            var webDir = Path.Combine(Directory.GetCurrentDirectory(), "Site");

            //------------------- define routes -------------------
            Route.Before = (rq, rp) => { Console.WriteLine($"Requested: {rq.Url.PathAndQuery}"); return false; };

            //home page
            Route.Add("/", (rq, rp, args) =>
            {
                rp.AsFile(rq, Path.Combine(webDir, "Index.html"));

            });


            Route.Add("/data.json", (rq, rp, args) =>
            {
                rp.AsText(_incubatorData, "application/json");
            });

            Route.Add((rq, args) =>
                {
                    args["file"] = Path.Combine(webDir, rq.Url.LocalPath.TrimStart('/'));
                    return Path.HasExtension(args["file"]);
                },
                (rq, rp, args) => rp.AsFile(rq, args["file"]));


            //home page
            Route.Add("/{param}", (rq, rp, args) =>
            {
                rp.AsFile(rq, Path.Combine(webDir, "Index.html"));

            });

            var port = 18888;
            Console.WriteLine("Running HTTP server on: " + port);

            var cts = new CancellationTokenSource();
            // C:\WINDOWS\system32 > netsh http add urlacl url = http://+:18888/ user=gl752vw\ag
            var ts = HttpServer.ListenAsync(port, cts.Token, Route.OnHttpRequestAsync, useHttps: false);
            //Application.ApplicationExit += delegate { AppExit.WaitFor(cts, ts); };
        }

        private void button5_Click(object sender, EventArgs e)
        {
            _formStatus = new FormStatus();
            _formStatus.Show(this);
        }
    }
}
