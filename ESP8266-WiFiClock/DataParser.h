#pragma once

#include <JsonListener.h>

class DataListener: public JsonListener {

  private:
    String currentKey;
    String currentParent = "";
    double temperature;
    String icon;
    int index = 0;
    int limit;
    long future_time[48];      // Array to store the time of future forecasts
    String future_summary[48]; // Array to store the summary of future forecasts
    String future_icon[48];    // Array to store the icon label of future forecasts
    bool enterArray = false;

  public:
    virtual void whitespace(char c);
    virtual void startDocument();
    virtual void key(String key);
    virtual void value(String value);
    virtual void endArray();
    virtual void endObject();
    virtual void endDocument();
    virtual void startArray();
    virtual void startObject();

    // Functions to retrieve parsed data
    void setForecastLimit(int n);
    double getTemperature();
    String getIcon();
    long getFutureTime(int i, int timezone);
    String getFutureSummary(int i);
    String getFutureIcon(int i);
};
