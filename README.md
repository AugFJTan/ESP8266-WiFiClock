# ESP8266 WiFi Clock

![WiFi Clock](/images/WiFiClock.jpg)

Utilizing the ESP8266's capability to retrieve the current time in the form of a Unix timestamp (a long-type number of how many seconds elapsed since the last epoch, i.e. January 1, 1970) from an NTP server, we are able to process the timestamp and output it in a human-readable form on a display screen. As if that wasn't enough, why don't we add a feature where we can view the latest weather information? With custom weather icons, no less? 

## Background

The design of the WiFi clock is largely inspired by Ubuntu, a Linux distro. The startup screen resembles that of the Ubuntu terminal, with the colour scheme and font to match. A weather API called [Dark Sky](https://darksky.net) is used to obtain weather data in the JSON format. As the ESP8266 has limited memory, a streaming parser is used to collect necessary data, instead of storing the entire JSON tree in memory.

## Hardware & Software Requirements

In terms of hardware, you will need:
* An ESP8266 module with enough free pins to support a screen output (a NodeMCU Lua ESP-12E WiFi Development Board mounted on a breadboard is used here)
* A display screen hooked up to the ESP8266 module (a 2.4" TFT LCD Touchscreen is used here)

As for software, you will need to install the [Arduino IDE](https://www.arduino.cc/en/main/software) on your computer. Once that's done, you will need to install the addon for the ESP8266. A quick guide on installing the addon can be found [here](https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon).

## Libraries Used

The libraries used here are third-party libraries you can download from Github. They are:
* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) - A graphics library for fonts and shapes.
* [Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341) - Display library for the LCD
* [XPT2046](https://github.com/spapadim/XPT2046) - Touch library for the LCD.
* [Json Streaming Parser](https://github.com/squix78/json-streaming-parser) - A streaming parser to parse JSON data.
* [Time](https://github.com/PaulStoffregen/Time) - For time-related calculations and functions.

## Weather API

The documentation for the Dark Sky API used in this project can be found [here](https://darksky.net/dev/docs). You will need to create an account and obtain a special API key. This key will be required when sending GET requests.

## Usage

1. Download or clone this repository into your desired directory. To clone using git, on the command line, type

```shell
git clone https://github.com/AugFJTan/ESP8266-WiFiClock
```
2. Place the files in the repository's `Fonts` folder into the actual `Fonts` folder in the `Adafruit_GFX` library folder.
3. Change the values in the `ESP8266-WiFiClock.ino` sketch where needed.
4. Compile the sketch and upload it to your ESP8266 module.

## Resources

* [Weather Icons (Piskel)](http://www.piskelapp.com/p/agxzfnBpc2tlbC1hcHByEwsSBlBpc2tlbBiAgICz7OTVCQw/view)
* [LOL face (Piskel)](http://www.piskelapp.com/p/agxzfnBpc2tlbC1hcHByEwsSBlBpc2tlbBiAgIDLpq3DCww/view)
* [ArraytoHex](/ArraytoHex)
* [Fonts](/Fonts)

## Further Reading

* [Converting fonts](docs/Convertingfonts.md)
* [Switching between different fonts](docs/Switchingbetweendifferentfonts.md)
* [Creating custom icons](docs/Creatingcustomicons.md)
