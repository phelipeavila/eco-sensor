#include "wifi_client.h"
#include "ESP8266.h"
#include <IRremote.h>
#include <EEPROM.h>



byte millisCycles = 0;
unsigned long lastTime = 0;
unsigned int dataNumber = 0;
boolean readSensor = true;
boolean memOverflow = false;
int eeAddress = 0;

struct DataObject
{
    unsigned int dataNum;
    int dataRead;
};

IRrecv irrecv(irPin);
decode_results results;


void setup()
{
    Serial.begin(9600); 
    irrecv.enableIRIn(); // Start the IR receiver

    esp8266_init(1); // Start the ESP8266

}
  

//------------------------------------------------------------------
//This function is responsible for 3 things:
//
//1 - Check if it's time to read the analog sensor. If it is, the variable
//'readSensor'will be true, the client will read a value and store in memory.
//
//2 - Read the sensor and save the value
//
//3 - Check if it received any signal from the host. If it did, It will try to
// connect to the host
//
//------------------------------------------------------------------
void loop()
{
    timeCheck(); 

    if (readSensor == true)
    {
        saveData (dataNumber, sensorRead());
        dataNumber += 1;
    }

    if (checkDroneSignal())
    {
        Serial.println("DRONE IS HERE!!!");
        //delay(2000);
        //esp8266_sendCmd("AT+CWQAP");
        //delay(1000);
        esp8266_sendCmd("AT+CWJAP=\"ArduinoHost\",\"1234567890\"");
        //delay(2000);
        sendData();
    }

}

//------------------------------------------------------------------
//Reads the IR receiver and checks for anny command received.
//I used a IR remote to test the client, and I assigned tasks for different
// buttons of the remote. But the host is configured to send only 
//the command '0xFFC23D' which corresponds to the button 'play' of the remote.
//
//If the client receive the the command '0xFFC23D',
//------------------------------------------------------------------
int checkDroneSignal ()
{
    if (irrecv.decode(&results))
    {
        switch (results.value)
        {
            case 0xFFA25D: //CH -
                esp8266_enable(0); //turn off the esp8266
                delay(100);
                irrecv.resume(); // Receive the next value
                break;
            case 0xFFE21D: //CH + 
                esp8266_enable(1); //turn on the esp8266
                delay(100);
                irrecv.resume(); // Receive the next value
                break;
            case 0xFF629D: //CH
                esp8266_enable(1);
                esp8266_sendCmd("AT+CWLAP"); //list all availables wifi networks around
                delay(100);
                irrecv.resume(); // Receive the next value
                break;
            case 0xFFC23D: //play
                esp8266_enable(1); //turn on esp8266
                delay(100);
                irrecv.resume(); // Receive the next value
                return 1;
                break;
            case 0xFF906F: //EQ
                Serial.println(getTimeOn());   //print the number of minutes since the arduino was turned on
                delay(100);
                irrecv.resume(); // Receive the next value
                break;

            default: // if the command received did not match any above, it will print the command
                if(results.value != 0)
                Serial.println(results.value, HEX);   
                delay(100);
                irrecv.resume(); // Receive the next value
                break;
        }
    }
    return 0;
}


//------------------------------------------------------------------
//Check if it's time to read the sensor.
//Every time the arduino reads the sensor, it saves the number of milliseconds
//since the arduino was turned on until the measurement. This function checks 
//if its time again comparing that number with the number of milliseconds
//since the arduino start running untill now.
// The frequency of measurements can be controlled with the constant DELAY_MS
//
//------------------------------------------------------------------
int timeCheck ()
{
    if ((millis() - lastTime) >= DELAY_MS)
    {  
        readSensor = true;
        lastTime = millis();

    //if the arduino is on for too long time, the number of milliseconds might be too long to be saved
    //in a unisgned long variable. If that rappends, it will start over. To avoid problems with the measurement
    //of the sensor, the code below will solve this problem
    } else if ((millis() - lastTime) < 0 && (((MAX_COUNT - lastTime) + millis()) >= DELAY_MS ))
    {
        readSensor = true;
        lastTime = millis();
        millisCycles += 1;

    }
}

//------------------------------------------------------------------
//returns the number of minutes since the arduino start running. Microcontrollers are not
//good in making divisions. Instead of making it divide the number of milliseconds,
// it is better to make it multiply it.
// 0.00016667 == 1/(1000*60)
//------------------------------------------------------------------
unsigned long getTimeOn ()
{   
    //every time the number of milliseconds of running time start over, 
    //the variable millisCycles will be increased by 1.
    //71582 is the number of minutes that will take to the number start counting again
    //
    return (millis()*0.00016667 + 71582*millisCycles);
}

//------------------------------------------------------------------
//reads the analog sensor
//------------------------------------------------------------------
int sensorRead ()
{   
    return analogRead(A2);
}

//------------------------------------------------------------------
//Save the data in EEPROM. Every time it saves one struct, it increases
//eeAddress to keep tracking of the position in memory the data is
//------------------------------------------------------------------
int saveData (unsigned int dataNum, int dataRead)
{
    DataObject data = {
        dataNum,
        dataRead
    };

    EEPROM.put(eeAddress, data);
    eeAddress += sizeof(DataObject);

    if (eeAddress > 255)
    {
        memOverflow = true;
        eeAddress = 0;
    }
    Serial.print("\nSaved: ");
    Serial.print(data.dataNum);
    Serial.print(" ,");
    Serial.println(data.dataRead);
    readSensor = false;
}

//------------------------------------------------------------------
//reads data from EEPROM to the host. every time it reads a value,
//it decreases eeAddress to keep tracking of the position in memory.
//When sending the data, the client first sends the DEVICE ID,
//add a comma, send how long ago (how long before the connection) that
//information was saved, add a comma, and add te data
//------------------------------------------------------------------
int sendData()
{
    if (esp8266_isConected() == 2)
    {
        esp8266_sendCmd("AT+CIPSTART=\"TCP\",\"192.168.1.1\",1001"); //open TCP connection
        unsigned long timeOn = getTimeOn();
        //do
        //Serial.print("eeAddress: ");
        Serial.println(eeAddress);
        delay(2000);
        while (eeAddress > 0)
        {
            eeAddress -= sizeof(DataObject);
            DataObject data;
            String command = String(DEVICE_ID) + "," ;
            EEPROM.get(eeAddress, data);


            command += String(timeOn - data.dataNum*DELAY_MIN) + ",";
            Serial.print("\nRead: ");
            Serial.print(timeOn - data.dataNum*DELAY_MIN);

            command += data.dataRead;
            Serial.print(" ,");
            Serial.println(data.dataRead);

            esp8266_sendCmd("AT+CIPSEND=" + String(command.length()));
            esp8266_sendCmd(command);
            Serial.println(command);
            delay(2000);

        } //while (eeAddress > 0);
        eeAddress = 0;
        esp8266_sendCmd("AT+CIPCLOSE"); // close TCP connection
        //esp8266_sendCmd("AT+CWQAP");
        esp8266_enable(0); //turn off esp8266
    }
}


 






