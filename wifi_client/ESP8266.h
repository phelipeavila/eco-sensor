#define wifiEnablePin 8

#include <SoftwareSerial.h>

SoftwareSerial wifiSerial(6, 5); // RX, TX

int esp8266_init(int mode);
int esp8266_enable (int en);
int esp8266_sendCmd (String msg);
int esp8266_isConected ();

//------------------------------------------------------------------
//Set up the module for the first use
//------------------------------------------------------------------
int esp8266_init(int mode)
{
    pinMode(wifiEnablePin, OUTPUT);
    //set baud rate
    //for default, the ESP8266 is set up to work at 115200 baud rate,
    //but the SoftwareSerial library does not work well at this 
    //baud rate. Because of that we change the speed to 9600
    
    esp8266_enable(1);
    wifiSerial.readString();
    delay(1000);
    wifiSerial.setTimeout(500);
    wifiSerial.begin(115200);
    wifiSerial.println("AT+CIOBAUD=9600");
    wifiSerial.readString();
    wifiSerial.begin(9600);
    esp8266_sendCmd("AT+CIOBAUD=9600");

    //This command sets the ESP8266 to work as a
    //1 - client
    //2 - Access Point mode (server)
    //3 - Client + AP
    esp8266_sendCmd("AT+CWMODE=" + String(mode));
    esp8266_enable(0);

}

//------------------------------------------------------------------
//Turns on(1) or off(0) the ESP8266 using the enable pin (default is 8)
//------------------------------------------------------------------
int esp8266_enable (int en)
{
    if (en == 0)
    { 
        digitalWrite(wifiEnablePin, LOW);
        return 0;
    } else if (en == 1)
    {
        digitalWrite(wifiEnablePin, HIGH);
        wifiSerial.readString();
        delay(1000);
        return 0;
    }
    else
        digitalWrite(wifiEnablePin, LOW);
        return 1;
}

//------------------------------------------------------------------
//Sends a command to ESP8266, waits for it to answer and prints the answer
//------------------------------------------------------------------
int esp8266_sendCmd (String msg)
{
    int waiting = 0;
    String resp = "";
    wifiSerial.println(msg);

    resp = wifiSerial.readString();
    Serial.print(resp); 

    while(!resp.endsWith("OK\r\n") && !resp.endsWith("ERROR\r\n") && !resp.endsWith("> ") )
    {
        resp = wifiSerial.readString();
        Serial.print(resp); 
        waiting = waiting + 1;

        if (waiting >= 10)
        {
            Serial.println("Error sending the command");
            return 1;
        }
          
    }

    return 0;
}

//------------------------------------------------------------------
//Check the connection status
//2 : Got IP
//3 : Connected
//4 : Disconnected
//5 : Wi-Fi connection fail
//------------------------------------------------------------------
int esp8266_isConected ()
{
	wifiSerial.println("AT+CIPSTATUS");
    
    return wifiSerial.readString().substring(22, 23).toInt();
}