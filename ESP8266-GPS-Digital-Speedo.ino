/*********************************************************************
The following is an ESP8266 Digital Speedo based on the Wemos D1 Mini and its matching 64x48 OLED sheild.
The tested GPS module was an U-blox NEO 6M part number GY-GPS6MV2.

origional code https://github.com/garywatts/ESP8266-GPS-Digital-Speedo

Required libraries include:
Adafruit Graphics
https://github.com/adafruit/Adafruit-GFX-Library
A modified version of Adafruit's SSD1306 library (This is important as the standard library won't display on our tiny diplay correctly)
https://github.com/mcauser/Adafruit_SSD1306
Tiny GPS++
http://arduiniana.org/libraries/tinygpsplus

Use this code at your own risk. 

Other Notes:
For Head Up Display (HUD) mode you could change line 247 of adafruit_ssd1306.ccp in your library folder as below
change ssd1306_command(SSD1306_COMSCANDEC); to ssd1306_command(SSD1306_COMSCANINC);

Notes on using different fonts https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

If you are alreading using the official Adafruit library for something else, this site below has a good tip on using the modified library 
as part of a wemos oled project.
https://diyprojects.io/shield-oled-wemos-d1-mini-ssd1306-64x48-pixels-review-esp-easy-adafruit_ssd1306

A shout out to these two projects for insperation and bits of code.
http://theelectromania.blogspot.it/2016/03/esp8266-esp-12e-nodemcu-and-ds18b20.html
https://github.com/mkconer/ESP8266_GPS/blob/master/ESP8266_GPS_OLED_Youtube.ino
 --------------------------------------------------------------------
 // following text is from standard adafruit library disclaimer
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using SPI to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>                        //choose font here & 2nd line of loop, try and keep same size for this display 
#include <TinyGPS++.h>                                  // Tiny GPS Plus Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module

static const int RXPin = 12, TXPin = 13;                // Ublox 6m GPS module to pins 12 (wemos D6) and 13 (wemos D7) 
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

int gps_speed;
int num_sat;

TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);                   // constructor to call OLED display using adafruit library

static const unsigned char PROGMEM logo_bmp[] =    // Below is the Ford Ka Logo.
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x07, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x78, 0x00, 0x00, 0x00,
0x00, 0x00, 0x1F, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x0F, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x3C, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xF8, 0x00, 0x00, 0x01, 0x00,
0x00, 0x00, 0xFF, 0xFC, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x01, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0x80,
0x00, 0x03, 0xFF, 0xFF, 0xFE, 0x07, 0xC3, 0x80, 0x00, 0x03, 0xFF, 0x00, 0x1F, 0x9F, 0x03, 0x80,
0x00, 0x07, 0xF8, 0x00, 0x03, 0xFC, 0x03, 0x80, 0x00, 0x0F, 0xF0, 0x00, 0x03, 0xF8, 0x03, 0xC0,
0x00, 0x1F, 0xC0, 0x00, 0x0F, 0xDE, 0x01, 0xC0, 0x00, 0x3F, 0x80, 0x00, 0x1F, 0x07, 0x81, 0xC0,
0x00, 0x3F, 0x00, 0x00, 0x7E, 0x01, 0xE1, 0xC0, 0x00, 0x7E, 0x00, 0x01, 0xF8, 0x00, 0x71, 0xC0,
0x00, 0xFC, 0x00, 0x03, 0xF0, 0x00, 0x1D, 0xE0, 0x01, 0xF8, 0x00, 0x07, 0xE0, 0x00, 0x07, 0xE0,
0x03, 0xF0, 0x00, 0x0F, 0xC0, 0x00, 0x03, 0xE0, 0x07, 0xF0, 0x00, 0x1F, 0x80, 0x00, 0x00, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//  Use someting like http://en.radzio.dxp.pl/bitmap_converter/ to create your own logo

void setup()   {                

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   
// display.setRotation(2);                            // uncomment for 180 display flip, may aid in mounting display etc

                                                      // init done

  display.display();                                  // Adafruit splashscreen from display buffer
  delay(1500);                                        // wait 1.5 secs before clear splash
  display.clearDisplay();
  
  // commenting out the above three lines would hide the adafruit splash screen (but you realy shouldn't)
  // same goes for below 4 lines to hide our custom ka logo, feel free to mod or get rid of this. 
   
  display.drawBitmap(0, 0, logo_bmp, 64, 48, 1);      // load our own splashscreen
  display.display();
  delay(1500);                                        // wait 1.5 secs before clear splash
  display.clearDisplay();
 
  ss.begin(GPSBaud);  
  }

void loop() {

  display.setTextColor(WHITE);  
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(2);                           // large font to display speed
 
   gps_speed = gps.speed.kmph();
    if (gps_speed < 10) {                           // if to center speed display below 10kph
    display.setCursor(20,26);                       
   }
   else if (gps_speed > 100) {                      // if to center speed display above 100kph
    display.setCursor(2,26);
   }
   else
   {
   display.setCursor(10,26);
   }
  display.println(gps_speed , DEC);
  display.setTextSize(1);                            // smaller font for kph
  display.setCursor(15,44);

  num_sat = gps.satellites.value();
 if (num_sat > 1) {                                 // if to provide subtle hint we have a gps fix
    display.println ("kp/h.");
   }
   else
   {
    display.println ("kp/h");
   }
  display.display();
  delay(200); 
  
  smartDelay(500);                                   // Run Procedure smartDelay

  if (millis() > 5000 && gps.charsProcessed() < 10)
    display.println(F("No GPS data received: check wiring"));
 display.clearDisplay();
}

static void smartDelay(unsigned long ms)           // This custom version of delay() ensures that the gps object is being "fed".
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
 }
