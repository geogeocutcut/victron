
// Enable debug prints
//#define MY_DEBUG

// Enable and select radio type attached
#define PHOENIX
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX

//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#define SKETCH_NAME "Phoenix Sensor"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define BVOLT_ID 1              // Id of the sensor child
#define CS_ID 2              // Id of the sensor child
#define MODE_ID 3              // Id of the sensor child
#define AC_OUT_V_ID 4              // Id of the sensor child
#define AC_OUT_I_ID 5              // Id of the sensor child

uint32_t TEMPO_TIME = 10000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
uint32_t ALL_TEMPO_TIME = 300000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.

#include <MySensors.h>
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial victronSerial(4, 5); // RX, TX

MyMessage childBVoltMsg(BVOLT_ID,V_VOLTAGE);
MyMessage childModeMsg(MODE_ID,V_VAR1);
MyMessage childCSMsg(CS_ID,V_VAR1);
MyMessage childOutVoltMsg(AC_OUT_V_ID,V_VOLTAGE);
MyMessage childOutCurrentMsg(AC_OUT_I_ID,V_CURRENT);

char receivedChars[buffsize];                       // 32 an array to store the received data
char tempChars[buffsize];                           // 32 an array to manipulate the received data

int32_t value[num_keywords]      = {0};                                                  
int32_t value_tmp[num_keywords]      = {0};                                              
int32_t value_old[num_keywords]      = {0};

bool new_data = false;
bool blockend = false;
byte checksum = 0;

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
    present(MODE_ID, S_POWER);
    delay(100);
    present(CS_ID, S_POWER);
    delay(100);
    present(AC_OUT_V_ID, S_MULTIMETER);
    delay(100);
    present(AC_OUT_I_ID, S_MULTIMETER);
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
        checksum += rc;

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
    char *strtokIndxLabel; 
    char *strtokIndxValue;
    
    strtokIndxLabel = strtok(tempChars,"\t");      // get the first part - the label
    // The last field of a block is always the Checksum
    if (strcmp(strtokIndxLabel, "Checksum") == 0) {
        blockend = true;
    }

    if (!blockend ) {
        // found the label, copy it to the value array
        for (int i = 0; i < num_keywords; ++i) {
            if (strcmp(strtokIndxLabel, keywords[i]) == 0) {
                strtokIndxValue = strtok(NULL, "\r");
                if (strtokIndxValue != NULL) {
                    value_tmp[i]=atol(strtokIndxValue);
                }
                break;
            }
        }
        
    }
    else
    {
        if(!checksum)
        {
            for(int i=0; i<num_keywords; ++i)
              value[i] = value_tmp[i];
            
            SendMySensorData();
        }
        else
        {
            for(int i=0; i<num_keywords; ++i)
              value_tmp[i] = value[i];
        }
        blockend = false;
        checksum = 0;
    }
}

void SendMySensorData() {
    static unsigned long prev_millis;
    static unsigned long prev_all_millis;
    if(millis() - prev_all_millis > ALL_TEMPO_TIME) {
        PrintAllValues();
        prev_all_millis = millis();
    }
    else if (millis() - prev_millis > TEMPO_TIME) {
        PrintValues();
        prev_millis = millis();
    }
}

void PrintValues() {
  
  float Batt_V = roundf(value[V] / 10.00f) / 100;
  float Batt_V_old = roundf(value_old[V] / 10.00f) / 100;
  if(Batt_V!= Batt_V_old)
  {
    send(childBVoltMsg.set(Batt_V,2));
    value_old[V]=value[V];
  }
  
  float Out_V = roundf(value[AC_OUT_V] / 1.00f) / 100;
  float Out_V_old = roundf(value_old[AC_OUT_V] / 1.00f) / 100;
  if(Out_V!= Out_V_old)
  {
    send(childOutVoltMsg.set(Out_V,2));
    value_old[AC_OUT_V]=value[AC_OUT_V];
  }
  
  float Out_I = roundf(value[AC_OUT_I] * 10.00f) / 100;
  float Out_I_old = roundf(value_old[AC_OUT_I] * 10.00f) / 100;  
  if(Out_I!= Out_I_old)
  {
    send(childOutCurrentMsg.set(Out_I,2));
    value_old[AC_OUT_I]=value[AC_OUT_I];
  }

  if(value[MODE]!= value_old[MODE])
  {
    send(childModeMsg.set(value[MODE]));
    value_old[MODE]=value[MODE];
  }
  if(value[CS]!= value_old[CS])
  {
    send(childCSMsg.set(value[CS]));
    value_old[CS]=value[CS];
  }
}

void PrintAllValues() {
  
  float Batt_V = roundf(value[V] / 10.00f) / 100;
  send(childBVoltMsg.set(Batt_V,2));
  value_old[V]=value[V];
  
  float Out_V = roundf(value[AC_OUT_V] / 1.00f) / 100;
  send(childOutVoltMsg.set(Out_V,2));
  value_old[AC_OUT_V]=value[AC_OUT_V];
  
  float Out_I = roundf(value[AC_OUT_I] * 10.00f) / 100;
  send(childOutCurrentMsg.set(Out_I,2));
  value_old[AC_OUT_I]=value[AC_OUT_I];

  send(childModeMsg.set(value[MODE]));
  value_old[MODE]=value[MODE];
  
  send(childCSMsg.set(value[CS]));
  value_old[CS]=value[CS];
}
