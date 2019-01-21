
// Enable debug prints
//#define MY_DEBUG
#define BMV_700

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX

//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#define SKETCH_NAME "BMV700 Sensor"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define BVOLT_ID 1              // Id of the sensor child
#define BCURRENT_ID 2              // Id of the sensor child
#define POWER_ID 2              // Id of the sensor child
#define CE_ID 3              // Id of the sensor child
#define TTG_ID 4              // Id of the sensor child
#define SOC_ID 5              // Id of the sensor child
#define DEEPEST_DISC_ID 6              // Id of the sensor child
#define LAST_DISC_ID 7              // Id of the sensor child
#define AVG_DISC_ID 8              // Id of the sensor child

uint32_t TEMPO_TIME = 10000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
uint32_t ALL_TEMPO_TIME = 300000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.

#include <MySensors.h>
#include <SoftwareSerial.h>
#include "config.h"

SoftwareSerial victronSerial(4, 5); // RX, TX

MyMessage childBVoltMsg(BVOLT_ID,V_VOLTAGE);
MyMessage childBCurrentMsg(BCURRENT_ID,V_CURRENT);
MyMessage childPowerMsg(POWER_ID,V_WATT);
MyMessage childCEMsg(CE_ID,V_CURRENT);
MyMessage childTTGMsg(TTG_ID,V_VAR1);
MyMessage childSOCMsg(SOC_ID,V_VAR1);
MyMessage childDeepDiscMsg(DEEPEST_DISC_ID,V_CURRENT);
MyMessage childLastDiscMsg(LAST_DISC_ID,V_CURRENT);
MyMessage childAvgDiscMsg(AVG_DISC_ID,V_CURRENT);

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
    present(POWER_ID, S_POWER);
    delay(100);
    present(CE_ID, S_MULTIMETER);
    delay(100);
    present(TTG_ID, S_POWER);
    delay(100);
    present(SOC_ID, S_POWER);
    delay(100);
    present(DEEPEST_DISC_ID, S_MULTIMETER);
    delay(100);
    present(LAST_DISC_ID, S_MULTIMETER);
    delay(100);
    present(AVG_DISC_ID, S_MULTIMETER);
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
    if (millis() - prev_millis > ALL_TEMPO_TIME) {
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

  float Batt_I = roundf(value[I] / 10.00f) / 100;
  float Batt_I_old = roundf(value_old[I] / 10.00f) / 100;  
  if(Batt_I!= Batt_I_old)
  {
    send(childBCurrentMsg.set(Batt_I,2));
    value_old[I]=value[I];
  }


  if(value[P]!= value_old[P])
  {
    send(childPowerMsg.set(value[P],2));
    value_old[P]=value[P];
  }

  float CE_I = roundf(value[CE] / 10.00f) / 100;
  float CE_I_old = roundf(value_old[CE] / 10.00f) / 100;  
  if(CE_I!= CE_I_old)
  {
    send(childCEMsg.set(CE_I,2));
    value_old[CE]=value[CE];
  }

  if(value[TTG]!= value_old[TTG])
  {
    send(childTTGMsg.set(value[TTG]));
    value_old[TTG]=value[TTG];
  }

  if(value[SOC]!= value_old[SOC])
  {
    send(childSOCMsg.set(value[SOC]));
    value_old[SOC]=value[SOC];
  }

  float H1_I = roundf(value[H1] / 10.00f) / 100;
  float H1_I_old = roundf(value_old[H1] / 10.00f) / 100;  
  if(H1_I!= H1_I_old)
  {
    send(childDeepDiscMsg.set(H1_I,2));
    value_old[H1]=value[H1];
  }
  
  float H2_I = roundf(value[H2] / 10.00f) / 100;
  float H2_I_old = roundf(value_old[H2] / 10.00f) / 100;  
  if(H2_I!= H2_I_old)
  {
    send(childLastDiscMsg.set(H2_I,2));
    value_old[H2]=value[H2];
  }

  float H3_I = roundf(value[H3] / 10.00f) / 100;
  float H3_I_old = roundf(value_old[H3] / 10.00f) / 100;  
  if(H3_I!= H3_I_old)
  {
    send(childAvgDiscMsg.set(H3_I,2));
    value_old[H3]=value[H3];
  }
}

void PrintAllValues() {
  
  float Batt_V = roundf(value[V] / 10.00f) / 100;
  send(childBVoltMsg.set(Batt_V,2));
  value_old[V]=value[V];

  float Batt_I = roundf(value[I] / 10.00f) / 100;
  send(childBCurrentMsg.set(Batt_I,2));
  value_old[I]=value[I];

  send(childPowerMsg.set(value[P],2));
  value_old[P]=value[P];

  float CE_I = roundf(value[CE] / 10.00f) / 100;
  send(childCEMsg.set(CE_I,2));
  value_old[CE]=value[CE];

  send(childTTGMsg.set(value[TTG]));
  value_old[TTG]=value[TTG];

  send(childSOCMsg.set(value[SOC]));
  value_old[SOC]=value[SOC];

  float H1_I = roundf(value[H1] / 10.00f) / 100;
  send(childDeepDiscMsg.set(H1_I,2));
  value_old[H1]=value[H1];
  
  float H2_I = roundf(value[H2] / 10.00f) / 100;
  send(childLastDiscMsg.set(H2_I,2));
  value_old[H2]=value[H2];

  float H3_I = roundf(value[H3] / 10.00f) / 100;
  send(childAvgDiscMsg.set(H3_I,2));
  value_old[H3]=value[H3];
}
