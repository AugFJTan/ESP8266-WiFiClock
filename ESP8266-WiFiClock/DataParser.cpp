#include "DataParser.h"
#include <JsonListener.h>

#define PARSER_DEBUG false  //Set to true to view parsed data via serial output. WARNING: may cause major latency if baud rate is low!

void DataListener::whitespace(char c) {
  if(PARSER_DEBUG) Serial.println("whitespace");
}

void DataListener::startDocument() {
  if(PARSER_DEBUG) Serial.println("start document");
}

void DataListener::key(String key) {
  if(PARSER_DEBUG) Serial.println("key: " + key);
  currentKey = String(key);
}

void DataListener::value(String value) {
  if(PARSER_DEBUG) Serial.println("value: " + value);

  if(currentParent == "currently")
  { 
    if(currentKey == "temperature")
      temperature = value.toFloat();

    if(currentKey == "icon")
      icon = value;
  }

  if(enterArray)
  {
      if(index > 0 && index < limit) // Skip the first hourly forecast (the same hour as the current hour)
      {
          if(currentKey == "time")
          future_time[index-1] = value.toInt();
                        
          if(currentKey == "summary")
          future_summary[index-1] = value;
                          
          if(currentKey == "icon")
          future_icon[index-1] = value;
      }
   }
  
}

void DataListener::endArray() {
  if(PARSER_DEBUG) Serial.println("end array. ");
  enterArray = false;
  index = 0;
}

void DataListener::endObject() {
  if(PARSER_DEBUG) Serial.println("end object. ");
  if(!enterArray) currentParent = "";
  else index++;
}

void DataListener::endDocument() {
  if(PARSER_DEBUG) Serial.println("end document. ");
}

void DataListener::startArray() {
   if(PARSER_DEBUG) Serial.println("start array. ");
   enterArray = true;
}

void DataListener::startObject() {
   if(PARSER_DEBUG) Serial.println("start object. ");
   currentParent = currentKey;
}

// Functions to retrieve parsed data
void DataListener::setForecastLimit(int n) {
    limit = n + 1;
}

double DataListener::getTemperature() {
    return temperature;
}

String DataListener::getIcon() {
    return icon;
}

long DataListener::getFutureTime(int i, int timezone) {
    return future_time[i] + timezone * 3600;
}

String DataListener::getFutureSummary(int i) {
    return future_summary[i];
}

String DataListener::getFutureIcon(int i) {
    return future_icon[i];
}
