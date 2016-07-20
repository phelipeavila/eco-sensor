#define wifiEnablePin 8
#include <SoftwareSerial.h>
#include <IRremote.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "wifi_host.h"

RTC_DS1307 rtc;
SoftwareSerial wifiSerial(6, 5); // RX, TX

void setup(){
    pinMode(wifiEnablePin, OUTPUT);
    pinMode(pushbutton, INPUT);
    analogWrite(A2, 0);
    analogWrite(A3, 1023);

	digitalWrite(wifiEnablePin, HIGH);
    delay(1000);
	
	Serial.begin(9600);
    wifiSerial.begin(9600);

    //starts TCP server
    wifiSerial.println("AT+CIPMUX=1");
    Serial.print(wifiSerial.readString());
    wifiSerial.println("AT+CIPSERVER=1,1001");
    Serial.println("WiFi OK");

    //starts SD card
    if (!SD.begin(10)) 
    {
        Serial.println("initialization failed!");
        return;
    }
    //test SD card
    File myFile = SD.open("test.txt", FILE_WRITE);
    if(myFile)
    {
        myFile.print("testtxt");
        Serial.println("SD OK");
    }else
    {
        Serial.println("SD FAIL");
    }
    //starts RTC
    if (! rtc.begin()) 
    {
        Serial.println("Couldn't find RTC");
        while (1);
    }
}

void loop () 
{
    IRsend irsend;

    //check for data comming from the esp8266
    if ( wifiSerial.available() )
    {
        String ans = wifiSerial.readString();
        Serial.println(ans);
        //if the strinf of data starts with 'IPD' it means data from TCP connection
        if (ans.startsWith("\r\n+IPD"))
        {
            removeHeader(ans); //removes the header (IPD)
            if(ans.endsWith("0,CLOSED\r\n")) //removes the tail
            {
                ans.remove(ans.length() - 10);
            }            
            
            //calculates when the data received was measured
            //TimeSpan(D,H,M,S)
            DateTime dtData = rtc.now() - TimeSpan(0,0,timeData (ans),0);
            //save data on SD card
            saveData(ans.substring(0,3), dtData, getData (ans));
        }
    }

    //check if the push button was pressed to send the IR command
    if (digitalRead(pushbutton)==0) //if (position==ok) send signal
    {
        for (int i = 0; i < 3; i++)
        {
            irsend.sendSony(0xFFC23D, 24);
            delay(40);
        }
        delay(5000);
    }
}
//------------------------------------------------------------------
//remove the header (IPD) from the received string
//------------------------------------------------------------------

int removeHeader (String &text)
{
    int index1 = 0;
    while(text.charAt(index1) != ':')
    {
        index1 += 1;
    }
    text = text.substring(index1 + 1);
    return 0;
}

//String getDeviceName (String &text)
//{
//    return text.substring(0,3);
//}

//------------------------------------------------------------------
//returns how long ago the data received was measured (in minutes)
//------------------------------------------------------------------
int timeData (String &text)
{
    int index1 = 0;
    int index2 = text.length();

    while(text.charAt(index1) != ',')
    {
        index1 += 1;
    }

    while(text.charAt(index2) != ',')
    {
        index2 -= 1;
    }

    return text.substring(index1 + 1, index2 + 1).toInt();
}

//------------------------------------------------------------------
//returns the data from the received string
//------------------------------------------------------------------
int getData (String &text)
{
    int i = text.length();
    while(text.charAt(i) != ',')
    {
        i -= 1;
    }
    return text.substring(i+1).toInt();
}

//------------------------------------------------------------------
//save in SD card
//------------------------------------------------------------------
int saveData (String name, DateTime dtData, int data)
{
    File myFile = SD.open("test.txt", FILE_WRITE);

    Serial.println("Writing SD");


    if (myFile)
    {
        myFile.print(name);
        myFile.print(",");
        myFile.printf("%d", dtData.year());
        myFile.printf("%02d", dtData.month());
        myFile.printf("%02d", dtData.day());
        myFile.printf("%02d", dtData.hour());
        myFile.printf("%02d", dtData.minute());
        myFile.print(",");
        myFile.println(data);
        Serial.println("done.");
        myFile.close();
    }else
    {
        // if the file didn't open, print an error:
        Serial.println("error");
    }

}