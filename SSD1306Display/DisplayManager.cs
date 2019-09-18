using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
//using Windows.Networking.Connectivity;
//using Windows.System.Threading;

namespace SSD1306Display
{
    
    public enum SSD1306_Addresses
    {
        SSD1306_Primaer = 0x3C,
        SSD1306_Secundar = 0x3D,
    };


   // [Windows.UI.Xaml.Data.Bindable]
    public sealed class DisplayManager
    {
    //    private  Display display;
        IDictionary<int, Display> m_Displays;

        public DisplayManager()
        {
            m_Displays = new Dictionary<int, Display>();
        }

        public void Dispose()
        {
            for (int i = 0; i < m_Displays.Count;i++)
            {
                Display d = m_Displays[i];
                d.Dispose();
            }

            m_Displays.Clear();


        }


        public Windows.Foundation.IAsyncOperation<Boolean> InitAsync(int Adress)
        {
            Task<Boolean> from = InitInternalAsync(Adress);

            Windows.Foundation.IAsyncOperation<Boolean> to = from.AsAsyncOperation();

            return to;

        }

        internal async Task<bool> InitInternalAsync(int Adress)
        {
            Display display = new SSD1306Display.Display();
            bool retvalue =  await display.Init((byte)Adress);
            if (retvalue)
            {
                m_Displays.Add(Adress, display);
            }
            return true;
        }

        public bool InitDisplay(int Adress)
        {
            bool keyPresent = m_Displays.ContainsKey(Adress);
            if (keyPresent)
            {
                Display display = m_Displays[Adress];
                bool isDeviceConnected = display.IsDeviceConnected();
                if (isDeviceConnected)
                {
                    return display.InitDisplay();
                }
                else
                {
                    display.Dispose();
                    m_Displays.Remove(Adress);
                }
            }

            return false;
        }

        public bool Update(int Adress, String line_1, String line_2, String line_3, String line_4)
        {
            bool keyPresent = m_Displays.ContainsKey(Adress);
            if (keyPresent)
            {
                Display display = m_Displays[Adress];
                display.ClearDisplayBuf();

                if (line_1.Length > 0) display.WriteLineDisplayBuf(line_1, 0, 0);
                if (line_2.Length > 0) display.WriteLineDisplayBuf(line_2, 0, 1);
                if (line_3.Length > 0) display.WriteLineDisplayBuf(line_3, 0, 2);
                if (line_4.Length > 0) display.WriteLineDisplayBuf(line_4, 0, 3);
                return display.DisplayUpdate();
            }

            return true;

        }


    }
}
