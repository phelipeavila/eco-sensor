#define pushbutton 2
//#define wifiEnablePin 8 - defined in ESP8266.h file

//headers
int removeHeader (String &text);
//String getDeviceName (String &text);
int timeData (String &text);
int getData (String &text);
int saveData (String name, DateTime dtData, int data);