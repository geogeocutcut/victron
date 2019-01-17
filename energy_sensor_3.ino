
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

uint32_t TEMPO_TIME = 10000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.

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
    if (millis() - prev_millis > TEMPO_TIME) {
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

  float Batt_I = roundf(value[I] / 10.00f) / 100;
  float Batt_I_old = roundf(value_old[I] / 10.00f) / 100;  
  if(Batt_I!= Batt_I_old)
  {
    send(childBCurrentMsg.set(Batt_I,2));
    value_old[I]=value[I];
  }

  float Panel_V = roundf(value[VPV] / 10.00f) / 100;
  float Panel_V_old = roundf(value_old[VPV] / 10.00f) / 100;
  if(Panel_V!= Panel_V_old)
  {
    send(childVoltMsg.set(Panel_V,2));
    value_old[VPV]=value[VPV];
  }
  
  if(value[PPV]!= value_old[PPV])
  {
    send(childWattMsg.set(value[PPV]));
    value_old[PPV]=value[PPV];
  }
  if(value[CS]!= value_old[CS])
  {
    send(childBCSMsg.set(value[CS]));
    value_old[CS]=value[CS];
  }
  if(value[H20]!= value_old[H20])
  {
    send(childTDWattMsg.set(value[H20]*10));
    value_old[H20]=value[H20];
  }
  if(value[H21]!= value_old[H21])
  {
    send(childMaxTDWattMsg.set(value[H21]));
    value_old[H21]=value[H21];
  }
  if(value[H22]!= value_old[H22])
  {
    send(childYTWattMsg.set(value[H22]*10));
    value_old[H22]=value[H22];
  }
  if(value[H23]!= value_old[H23])
  {
    send(childMaxYTWattMsg.set(value[H23]));
    value_old[H23]=value[H23];
  }
}
