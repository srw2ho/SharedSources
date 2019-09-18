// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Diagnostics;
using DisplayFont;
using Windows.Devices;

using Windows.Devices.Enumeration;
using Windows.Devices.I2c;

using Microsoft.IoT.Lightning.Providers;
using System.Threading.Tasks;

namespace SSD1306Display
{


    class Display
    {

        /* This driver is intended to be used with SSD1306 based OLED displays connected via I2c
      * Such as http://amzn.to/1Urepy1, http://amzn.to/1VG54aU and http://www.adafruit.com/product/938 
      * It will require four wires to the display.  Power (VCC), Ground, SCL and SDA (I2C)
      *
      * For a Raspberry Pi this is:
      * VCC -> Pi Pin 1 (3.3v)
      * Ground -> Pi Pin 6
      * SDA -> Pi Pin 3
      * SCL -> Pi Pin 5
      * 
      * For DIYMall branded OLEDs the I2C address is 0x3C.  It may vary by manufacturer and can be changed down below.
      */

        private const UInt32 SCREEN_WIDTH_PX = 128;                         /* Number of horizontal pixels on the display */
        private const UInt32 SCREEN_HEIGHT_PX = 64;                         /* Number of vertical pixels on the display   */
        private const UInt32 SCREEN_HEIGHT_PAGES = SCREEN_HEIGHT_PX / 8;    /* The vertical pixels on this display are arranged into 'pages' of 8 pixels each */
        private byte[,] DisplayBuffer =
            new byte[SCREEN_WIDTH_PX, SCREEN_HEIGHT_PAGES];                 /* A local buffer we use to store graphics data for the screen                    */
        private byte[] SerializedDisplayBuffer =
            new byte[SCREEN_WIDTH_PX * SCREEN_HEIGHT_PAGES];                /* A temporary buffer used to prepare graphics data for sending over i2c          */

        /* Definitions for I2C */
        private const string I2CControllerName = "I2C1";
        private readonly I2cDevice displayI2c = null;
        private const byte SSD1306_Address = 0x3C;

        /* Display commands. See the datasheet for details on commands: http://www.adafruit.com/datasheets/SSD1306.pdf                      */
        private readonly byte[] CMD_DISPLAY_OFF = { 0xAE };              /* Turns the display off                                    */
        private readonly byte[] CMD_DISPLAY_ON = { 0xAF };               /* Turns the display on                                     */
        private readonly byte[] CMD_CHARGEPUMP_ON = { 0x8D, 0x14 };      /* Turn on internal charge pump to supply power to display  */
        private readonly byte[] CMD_MEMADDRMODE = { 0x20, 0x00 };        /* Horizontal memory mode                                   */
        private readonly byte[] CMD_SEGREMAP = { 0xA1 };                 /* Remaps the segments, which has the effect of mirroring the display horizontally */
        private readonly byte[] CMD_COMSCANDIR = { 0xC8 };               /* Set the COM scan direction to inverse, which flips the screen vertically        */
        private readonly byte[] CMD_RESETCOLADDR = { 0x21, 0x00, 0x7F }; /* Reset the column address pointer                         */
        private readonly byte[] CMD_RESETPAGEADDR = { 0x22, 0x00, 0x07 };/* Reset the page address pointer                           */


        /* Initialize GPIO, I2C, and the display 
           The device may not respond to multiple Init calls without being power cycled
           so we allow an optional boolean to excuse failures which is useful while debugging
           without power cycling the display */

        public async Task<bool> Init(int DeviceAddress)
        {
            return true;
        }

        public void DisplayUpdate()
        {
             
        }
        public void WriteLineDisplayBuf(String Line, UInt32 Col, UInt32 Row)
        {

        }

        /* Sets all pixels in the screen buffer to 0 */
        public void ClearDisplayBuf()
        {

        }
    }

}
