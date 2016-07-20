#define DEVICE_ID 	101 //keep always 3 digits
#define DELAY_MIN	1 //Delay between measurements in minutes (MIN)*6000
#define DELAY_MS 	DELAY_MIN*6000
unsigned long MAX_COUNT = 4294967295; //2ˆ32-1  -> max unsigned long number

//Pin configuration
#define irPin 9
//#define wifiEnablePin 8 -- defined in the ESP8266.h file

//funcion headers
int checkDroneSignal ();
int timeCheck ();
unsigned long getTimeOn ();
int sensorRead ();
int saveData (short dataNum, short dataRead);
int sendData ();

