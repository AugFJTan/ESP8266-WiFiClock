//WiFi Clock with Weather Forecast
//by Augustine Tan

#include <TimeLib.h> 
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <WiFiUdp.h>
#include <XPT2046.h>
#include <WiFiClientSecure.h>
#include <JsonStreamingParser.h>
#include "DataParser.h"
#include <Fonts/UbuntuMono_B9pt7b.h>    //Ubuntu Mono Bold 9pt
#include <Fonts/UbuntuMono_R48pt7b.h>   //Ubuntu Mono Regular 48pt
#include <Fonts/UbuntuMono_R14pt7b.h>   //Ubuntu Mono Regular 14pt
#include <Fonts/Weather32pt7b.h>        //Weather icons
 
// NTP details
unsigned int localPort = 2390;         // Local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28);  // time.nist.gov NTP server. Change IP to access a different server.
const int timeZone = 0;                // Set this to your timezone relative to UTC
const int NTP_PACKET_SIZE = 48;        // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];    // Buffer to hold incoming and outgoing packets
WiFiUDP udp;

// Change these values with your screen module
#define TFT_DC     3
#define TFT_CS    15
#define TFT_RESET  5

// Use hardware SPI (on ESP82661) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/0);

#define SSID "......"
#define SSID_PASSWORD "......"

// Define colours for custom startup screen
#define WFC_UBUNTUPURPLE 0x714A
#define WFC_LIMEGREEN    0x07E0
#define WFC_SKYBLUE      0x7E1D

// Dark Sky API Details 
WiFiClientSecure client;  // Instance of secure client to enable HTTPS request
const char* host = "api.darksky.net";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "......";

String api_key = "......"; // Obtain this from your Dark Sky account
String longitude = "......"; // Set both longitude and latitude to at least 4 decimal places
String latitude = "......";
String units = "si";

bool initialized = false; // Initialization flag

// Parser instances
JsonStreamingParser parser;
DataListener listener;

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
    Serial.println("Sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

time_t getNtpTime()
{
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Received NTP Response");
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(ILI9341_RED);
  tft.fillScreen(ILI9341_GREEN);
  tft.fillScreen(ILI9341_BLUE);
  tft.fillScreen(ILI9341_BLACK);
  return micros() - start;
}

void setup() { 
  Serial.begin(230400);
  
  //TFT RESET
  pinMode(TFT_RESET, OUTPUT);
  digitalWrite(TFT_RESET, 0) ;    // reset the LCD just in case it got hung
  delay (500);
  digitalWrite(TFT_RESET, 1) ;     
  delay (500);

  //2.4" TFT LCD initialisation
  tft.begin();
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  Serial.println(F("Benchmark                Time (microseconds)"));

  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
  delay(500);

  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation

  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());

  // Replace these for your screen module
  touch.setCalibration(1851,1731,308,232);

  tft.setRotation(3);
  touch.setRotation(XPT2046::ROT270);

  Serial.println("Displaying startup screen.");
  
  tft.fillScreen( WFC_UBUNTUPURPLE );
  
  tft.setFont(&UbuntuMono_B9pt7b);
  tft.setCursor(0, 16);
  tft.setTextColor(WFC_LIMEGREEN); //Green text
  tft.print("myname@mymachine:");
  tft.setTextColor(WFC_SKYBLUE); //Sky blue text
  tft.print("~");
  tft.setTextColor(ILI9341_WHITE);
  tft.print("$ ./WiFiClock\n");
   
  tft.printf("Connecting to %s... ", SSID);
  Serial.printf("Connecting to %s\n", SSID);
   
  WiFi.begin(SSID, SSID_PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(SSID, SSID_PASSWORD);
      delay(500);
      Serial.print(".");
  }

  tft.print("Connected\n");
   
  tft.printf("Starting UDP at %d... ", localPort);
  Serial.println("\nStarting UDP");
  udp.begin(localPort);
  tft.print("Started\n");
    
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  setSyncProvider(getNtpTime);

  tft.print("Requesting time from NTP server... ");

  int count = 0;
  while (timeStatus() == timeNotSet)
  {
    delay(50);
      
    if (count == 100)
    {
        tft.print("\nHmm... This is taking longer than \nusual. You might want to restart.");
        count++;
    }
    else if (count < 100)
        count++;
  } 
   
  tft.print("Retrieved\n");
  tft.print("Current time: ");
  printDigits(hour());
  tft.print(":");
  printDigits(minute());
  tft.print(" ");
  tft.print(day());
  tft.print(".");
  tft.print(month());
  tft.print(".");
  tft.print(year());

  apiCallDelay();

  tft.print("\nRequesting weather information from DarkSky.net... ");
  retrieveWeather();
}

time_t prevMoment = -1, prevMinute = -1, prevDay = -1;

int page = 1;
boolean api_flag = false;
int api_call_min, api_call_hour;
bool next_page = false;
bool updateFutureForecast = false, displayingForecast = false;
int limit = 6; //Number of future hourly forecasts to assign in memory

void loop() {

 uint16_t x, y;
 
  if (touch.isTouching()) {
    touch.getPosition(x, y);

    Serial.print("\nTouch detected.");
    
    next_page = true;
  } else {
    next_page = false;
  }

  if(next_page) // Switch 'pages' from 'Home()' to 'Forecast' and vice versa
  {
      if (page == 1) page = 2;
      else page = 1;
      tft.fillScreen(ILI9341_BLACK);
  }

  if (page == 1) 
  {
      Home();  
      if(displayingForecast) 
        displayCurrentWeather();
      
      displayingForecast = false;
  }
  if (page == 2) 
      Forecast();
      
  if(hour() == api_call_hour && minute() == api_call_min)
  {
      if(api_flag == false)
      {
          api_flag = true;
          apiCallDelay();
          retrieveWeather();
      }
  } else
      api_flag = false;
}

// digital clock display of the time
void digitalClockDisplay(){ 
  if(initialized)
      tft.fillRect(0, 55, 237, 80, ILI9341_BLACK); //Clear Hour and Minute
  else
  {
      prevMinute = minute();
      initialized = true;
  }
    
  if(day() != prevDay)
  {
      tft.fillRect(0, 140, 237, 33, ILI9341_BLACK); //Clear Day and Date
      prevDay = day();
  }
  
  tft.setTextSize(1);
  tft.setTextColor( ILI9341_GREENYELLOW);

  tft.setFont(&UbuntuMono_R14pt7b);
  tft.setCursor(5, 165);

  Serial.println(); 
  
  DayOfWeek(weekday()); Serial.print(" ");
  tft.print(", ");
  tft.print (day()); Serial.print(day());
  tft.print (" ");  Serial.print(" ");    
  Month(month());
  tft.print (" ");  Serial.print(" ");   
  tft.print (year());  Serial.print(year());
  Serial.print(" ");  

  tft.setFont(&UbuntuMono_R48pt7b);
  tft.setCursor(0, 125);
  
  printDigits(hour());
  Serial.print(":");
  tft.fillCircle(118, 90, 7, ILI9341_GREENYELLOW);
  tft.fillCircle(118, 120, 7, ILI9341_GREENYELLOW);
  tft.print (" ");
  printDigits(minute());
     
  Serial.println(); 
}

// Print an extra zero in front if the digit is less than 10
void printDigits(int digits){
  if(digits < 10)
  {
    Serial.print('0');
    tft.print("0");
  }
  Serial.print(digits);
  tft.print (digits);
}

void DayOfWeek(int Day){
  if (Day == 1) {tft.print("Sun"); Serial.print("Sun");}
  else if (Day == 2) {tft.print("Mon"); Serial.print("Mon");}
  else if (Day == 3) {tft.print("Tue"); Serial.print("Tue");}
  else if (Day == 4) {tft.print("Wed"); Serial.print("Wed");}
  else if (Day == 5) {tft.print("Thu"); Serial.print("Thu");}
  else if (Day == 6) {tft.print("Fri"); Serial.print("Fri");}
  else if (Day == 7) {tft.print("Sat"); Serial.print("Sat");}
}

void Month(int _month){
  if (_month == 1) {tft.print("Jan"); Serial.print("Jan");}
  else if (_month == 2) {tft.print("Feb"); Serial.print("Feb");}
  else if (_month == 3) {tft.print("Mar"); Serial.print("Mar");}
  else if (_month == 4) {tft.print("Apr"); Serial.print("Apr");}
  else if (_month == 5) {tft.print("May"); Serial.print("May");}
  else if (_month == 6) {tft.print("Jun"); Serial.print("Jun");}
  else if (_month == 7) {tft.print("Jul"); Serial.print("Jul");}
  else if (_month == 8) {tft.print("Aug"); Serial.print("Aug");}
  else if (_month == 9) {tft.print("Sep"); Serial.print("Sep");}
  else if (_month == 10) {tft.print("Oct"); Serial.print("Oct");}
  else if (_month == 11) {tft.print("Nov"); Serial.print("Nov");}
  else if (_month == 12) {tft.print("Dec"); Serial.print("Dec");} 
}

void retrieveWeather(){
  if (!client.connect(host, httpsPort)) {
      Serial.println("Connection failed.");
      return;
  }

  if (client.verify(fingerprint, host)) {
      Serial.println("Certificate matches.");
  } else {
      Serial.println("Certificate doesn't match.");
  }

  String exclude = "";

  // Hourly data is excluded when future forecast data is not needed; however, it is included at initialization and every 15 minutes
  if(initialized && minute() != 0 && minute() % 15 != 0)
  {
      exclude += "hourly,";
  }
  else if(displayingForecast) 
      updateFutureForecast = true; 
   
  exclude += "minutely,daily,alerts,flags";
   
  String url = "https://api.darksky.net/forecast/" + api_key + "/" + longitude + "," + latitude + "?exclude=" + exclude + "&units=" + units;

  // Send GET request to api.darksky.net
  Serial.println("Sending GET request to api.darksky.net...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
 
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received.");
      break;
    }
  }

  // Limit the number of hourly forecast data stored in memory
  listener.setForecastLimit(limit);

  Serial.println(String(ESP.getFreeHeap()));
  parser.setListener(&listener);

  boolean isBody = false;
  char c;
  int size = 0, index = 0;
  Serial.print("Reading and parsing data from api.darksky.net...");

  // Parse the JSON data retrieved from the server
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
  Serial.println(" Done.");
  Serial.println(String(ESP.getFreeHeap()));

  // Reset the parser so that it starts from the beginning the next time around
  parser.reset();
  
  if(!initialized)
  {
     tft.print("Retrieved\n");
     tft.fillScreen( ILI9341_BLACK );
     digitalClockDisplay(); 
  }

  if(!displayingForecast) displayCurrentWeather();

  client.stop();
}

void displayCurrentWeather()
{
  tft.fillRect(250, 5, 64, 64, ILI9341_BLACK ); // Clear weather icon
  
  tft.setTextColor( ILI9341_GREENYELLOW);
  tft.setTextSize(2);  // Enlarge the weather icon (font) by 2X
 
  String icon = listener.getIcon();

  printWeatherIcon(250, 5, icon);

  tft.fillRect(245, 69, 70, 20, ILI9341_BLACK );
  tft.setTextSize(1);
  tft.setFont(&UbuntuMono_B9pt7b);
  tft.setCursor(255, 84);
  char temp[20];
  dtostrf(listener.getTemperature(), 4, 1, temp); // Format floating point temperature value to one decimal place
  tft.printf("%s C", temp);

  tft.drawCircle(295, 76, 2, ILI9341_GREENYELLOW); // Draw degree symbol
}

void printWeatherIcon(int x, int y, String icon)
{
  tft.setFont(&Weather32pt7b);
  tft.setCursor(x, y);

  if(icon == "clear-day")
      tft.print(0);
  else if(icon == "partly-cloudy-day")
      tft.print(1);
  else if(icon == "clear-night")
      tft.print(2);
  else if(icon == "partly-cloudy-night")
      tft.print(3);
  else if(icon == "cloudy")
      tft.print(4);
  else if(icon == "rain")
      tft.print(5);
  else if(icon == "wind")
      tft.print(6);
  else
  {
    tft.setTextSize(1);
    tft.setFont(&UbuntuMono_B9pt7b);
    tft.setCursor(x, y+10);
    tft.setTextWrap(false);
    tft.print(icon);
  }

  tft.setFont(); // Reset font to default
  tft.setTextWrap(true);
}

void apiCallDelay() {
  if(!initialized) // Initialize the next API call to the nearest 'multiple-of-five' minute, i.e. 5, 10, 15, 20, etc.
  {
    int setMinute = minute();
    
    if(setMinute % 10 >= 5)
        api_call_min = setMinute + (10 - setMinute % 10);
    else 
        api_call_min = setMinute + (5 - setMinute % 10);
  }
  else
  {
    api_call_min = minute() + 5;
  }

  api_call_hour = hour();
  
  if (api_call_min == 60){
    api_call_min = 0;
    api_call_hour += 1;
    if (api_call_hour == 24) api_call_hour = 0; // Set hour to midnight
  }
}

void Home(){
  if (timeStatus() != timeNotSet) {
    if (minute() != prevMinute) // Update the display only if time has changed 
    { 
        prevMinute = minute();
        digitalClockDisplay(); 
    }

    if(now() != prevMoment) // Update the second
    {
        tft.fillRect(245, 107, 50, 30, ILI9341_BLACK );
        prevMoment = now();
        tft.setTextSize(1);
        tft.setFont(&UbuntuMono_R14pt7b);
        tft.setCursor(255, 127);
        printDigits(second());
        Serial.print(" ");
    } 
  }  
}

void Forecast()
{
  // Set the values to negative prompt the display to update when we return to the 'Home' page
  prevMinute = -1;
  prevMoment = -1;

  if(updateFutureForecast) displayingForecast = false; // Reset what is displayed when there is an update on forecasts
  
  if(!displayingForecast)
  {
    Serial.println("\nDisplaying weather forecast data."); 
    displayingForecast = true;
    tft.setTextSize(1);
    tft.setFont(&UbuntuMono_R14pt7b);
    tft.setCursor(20, 40);
    tft.print("Forecasts:");

    tft.drawRect(188, 13, 120, 37, ILI9341_GREENYELLOW); 
    tft.setFont(&UbuntuMono_B9pt7b); 
    tft.setCursor(190, 28);
    tft.print("Powered by");
    tft.setCursor(190, 43);
    tft.print("Dark Sky");
    tft.setFont(&Weather32pt7b);
    tft.setCursor(285, 17);
    tft.print(7); // Print Dark Sky logo

    tft.drawFastHLine(15, 113, 285, ILI9341_GREENYELLOW);
    tft.drawFastHLine(15, 170, 285, ILI9341_GREENYELLOW);
    tft.drawFastVLine(157, 60, 167, ILI9341_GREENYELLOW);

    // Print 2X3 table of future forecasts
    int x_shift = 0;
    int index = 0;
    for(int i=0; i<2; i++)
    {
      for(int j=0; j<3; j++)
      {
        if(updateFutureForecast) tft.fillRect(15+x_shift, 60+(j*58), 140, 50, ILI9341_BLACK);
        tft.setFont(&UbuntuMono_B9pt7b); 
        tft.setCursor(20+x_shift, 75+(j*58));
        printDigits(hour(listener.getFutureTime(index, timeZone)));
        tft.print(":00"); 
        tft.setCursor(20+x_shift, 105+(j*58));
        tft.print(listener.getFutureSummary(index));
        printWeatherIcon(110+x_shift, 60+(j*58), listener.getFutureIcon(index));
        index++;
      }
      x_shift = 150;  
    }
    updateFutureForecast = false;
  }
}
