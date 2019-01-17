
// Enable debug prints
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX

//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#define SKETCH_NAME "Power Sensor"
#define SKETCH_MAJOR_VER "3"
#define SKETCH_MINOR_VER "0"

#define BVOLT_ID 1              // Id of the sensor child
#define BCURRENT_ID 2              // Id of the sensor child
#define CS_ID 3              // Id of the sensor child
#define VOLT_ID 4              // Id of the sensor child
#define POWER_ID 5              // Id of the sensor child
#define TDPOWER_ID 6              // Id of the sensor child
#define MAXTDPOWER_ID 7              // Id of the sensor child
#define YTPOWER_ID 8              // Id of the sensor child
#define MAXYTPOWER_ID 9              // Id of the sensor child

//uint32_t SLEEP_TIME = 30000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.

#include <MySensors.h>
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial victronSerial(4, 5); // RX, TX

MyMessage childBVoltMsg(BVOLT_ID,V_VOLTAGE);
MyMessage childBCurrentMsg(BCURRENT_ID,V_CURRENT);
MyMessage childBCSMsg(CS_ID,V_VAR1);
MyMessage childVoltMsg(VOLT_ID,V_VOLTAGE);
MyMessage childWattMsg(POWER_ID,V_WATT);
MyMessage childTDWattMsg(TDPOWER_ID,V_WATT);
MyMessage childMaxTDWattMsg(MAXTDPOWER_ID,V_WATT);
MyMessage childYTWattMsg(YTPOWER_ID,V_WATT);
MyMessage childMaxYTWattMsg(MAXYTPOWER_ID,V_WATT);

char receivedChars[buffsize];                       // 32 an array to store the received data
char tempChars[buffsize];                           // 32 an array to manipulate the received data
                                                    
char value[num_keywords][value_bytes]       = {0};  // 19 * 32 The array that holds the verified data

static byte blockindex = 0;
bool new_data = false;
bool blockend = false;


void setup()
{
  victronSerial.begin(19200);
}

void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER "." SKETCH_MINOR_VER);

  // Register this device as power sensor
  present(BVOLT_ID, S_MULTIMETER);
  delay(100);
  present(BCURRENT_ID, S_MULTIMETER);
  delay(100);
  present(CS_ID, S_POWER);
  delay(100);
  present(VOLT_ID, S_MULTIMETER);
  delay(100);
  present(POWER_ID, S_POWER);
  delay(100);
  present(TDPOWER_ID, S_POWER);
  delay(100);
  present(MAXTDPOWER_ID, S_POWER);
  delay(100);
  present(YTPOWER_ID, S_POWER);
  delay(100);
  present(MAXYTPOWER_ID, S_POWER);
  delay(100);

}

void loop()
{
    // Receive information on Serial from MPPT
    RecvWithEndMarker();
    HandleNewData();
}

void RecvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (victronSerial.available() > 0 && new_data == false) {
        rc = victronSerial.read();
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= buffsize) {
                ndx = buffsize - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            new_data = true;
        }
    }
}

void HandleNewData() {
    // We have gotten a field of data 
    if (new_data == true) {
        //Copy it to the temp array because parseData will alter it.
        strcpy(tempChars, receivedChars);
        ParseData();
        new_data = false;
    }
}

void ParseData() {
    char * strtokIndxLabel; // this is used by strtok() as an index
    char * strtokIndxValue;
    
    strtokIndxLabel = strtok(tempChars,"\t");      // get the first part - the label
    // The last field of a block is always the Checksum
    if (strcmp(strtokIndxLabel, "Checksum") == 0) {
        blockend = true;
    }
    

    if (blockend) {
        // Mettre la valeur en cours dans values
        for (int i = 0; i < num_keywords; ++i) {
          if (strcmp(strtokIndxLabel, keywords[i]) == 0) {
            // found the label, copy it to the value array
            strtokIndxValue = strtok(NULL, "\r");
            if (strtokIndxValue != NULL) {
              strcpy(value[i], strtokIndxValue);
            }
            break;
          }
        }
 
        // Reset the block index, and make sure we clear blockend.
        blockend = false;
        SendMySensorData();
    }
}

void SendMySensorData() {
    static unsigned long prev_millis;
    if (millis() - prev_millis > 10000) {
        PrintValues();
        prev_millis = millis();
    }
}


void PrintValues() {
  
  float Batt_V = roundf(atol(value[V]) * 100) / 100;
  send(childBVoltMsg.set(Batt_V,2));
  
  float Batt_I = roundf(atol(value[I]) * 100) / 100;
  send(childBCurrentMsg.set(Batt_I,2));

  float Panel_V = roundf(atol(value[VPV]) * 100) / 100;
  send(childVoltMsg.set(Panel_V,2));
  
  send(childWattMsg.set(atol(value[PPV])));
  
  send(childBCSMsg.set(atol(value[CS])));
  
  send(childTDWattMsg.set(atol(value[H20])*10));
  send(childMaxTDWattMsg.set(atol(value[H21])));
  send(childYTWattMsg.set(atol(value[H22])*10));
  send(childMaxYTWattMsg.set(atol(value[H23])));
}
