/**
 * @file main.ino
 * @author xdubx 
 * @brief 
 * @version 0.1
 * @date 2021-11-15
 * 
 * @copyright MIT 2021
 * 
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Wire.h"
#include "Arduino.h"

/**
 * @brief set the workmode of the esp
 *  0 = api endpoint over http
 *  1 = send the values
 */

#define workModeAPI true;
#define workModeSEND false;
#define debug false;

// requests
const String keys[] = {
    "gridVoltage",
    "gridCurrent",
    "gridPower",
    "pv1Voltage",
    "pv1Current",
    "pv2Voltage",
    "pv2Current",
    "pv1Power",
    "pv2Power",
    "gridFrequency",
    "mode",
    "eTotal",
    "eToday",
    "temp",
    "runTime"};

const unsigned char usbRegister[] = {0xaa, 0x55, 0x11, 0x02, 0x01, 0x53, 0x57, 0x41, 0x4D, 0x54, 0x4C, 0x59, 0x34, 0x5A, 0x4D, 0x1f, 0x04};
const unsigned char requestSerial[] = {0xaa, 0x55, 0x11, 0x02, 0x01, 0x53, 0x57, 0x41, 0x4D, 0x54, 0x4C, 0x59, 0x34, 0x5A, 0x4D, 0x1f, 0x04};
const unsigned char requestData[] = {0xaa, 0x55, 0x07, 0x01, 0x05, 0x0c, 0x01};
// END requests

//GLOBAL VARS
unsigned char message[200];
int count = 0;
const char *ssid = "your wifi here";
const char *password = "your wifi pw here";
const char *host = "url here"; // example http://192.168.1.1:8085/hello

//END GLOBAL VARS

void setup()
{
    Serial.begin(9600);

#ifdef debug
    //TODO: swap the uard?
#endif

#ifdef workModeAPI
    // create webserver

#endif

#ifdef workModeSEND

    bool wifi = createConnectionToWifi();
    if (wifi)
    {
        // register dongle

        // pull data
        registerDongle();
        memset(message, 0, sizeof(message));

        sleep(1);
        requestInverterData();
        sendRequest();
        //     // send to endpoint
        ESP.deepSleep(216000000000); // for ~1h
    }
    else
    {
        // go sleep or make something that the wifi is broke
    }

#endif

    // flush dataholder
    memset(message, 0, sizeof(message));
}

void loop()
{
#ifdef workModeAPI
    //loop webserver

#endif
}

// wifi related stuff

/**
 * @brief Create a Connection To Wifi
 * 
 * @return true on success
 * @return false on fail
 */
bool createConnectionToWifi()
{
    // Connect to WiFi network

#ifdef debug
    Serial.println("-------------------");
    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    byte tCounter = 0;
    while (tCounter < 60)
    {
        delay(1000);
        Serial.println(WiFi.status());
        // TODO break while for error if dont connect after 1 min
        if (WiFi.status() == WL_CONNECTED)
        {
#ifdef debug
            Serial.println("");
            Serial.println("WiFi connected: ");
            Serial.print(WiFi.localIP());
            Serial.println();
#endif
            return true;
        }
        tCounter++;
    }
    if (tCounter >= 60)
    {
        //TODO:  when tConter is over 60 sec a wifi error apears
#ifdef debug
        Serial.println("WLAN failed");
#endif
        return false;
    }
}

void sendRequest()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String msg = decodeInverterRes();
        http.begin(host); //TODO: handle not reachable error
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(msg);
#ifdef debug
        Serial.println(httpCode);
#endif
        //TODO: handle error
    }
    else
    {
#ifdef debug
        Serial.println("Error in WiFi connection");
#endif
        //TODO: handle error
    }
}

// END

bool getInverterSerial()
{
    // request serial of inverter
    Serial.print(requestSerial);
    count = 0;

    while (Serial.available() > 0 && count < 40)
    {
        message[count] = Serial.read();
        count++;
    }
    // parse response
#ifdef debug
    Serial.println(message);
#endif
}

/**
 * @brief register the usb dongle on the inverter
 * 
 * @return true if register was successfull
 * @return false register failed
 */
bool registerDongle()
{
    Serial.print(usbRegister);
    count = 0;
    while (Serial.available() > 0 && count < 10)
    {
        message[count] = Serial.read();
        count++;
    }
#ifdef debug
    Serial.println(message);
#endif
    // check message
}

bool requestInverterData()
{
    Serial.print(requestData);
    count = 0;
    while (Serial.available() > 0 && count < 200)
    {
        message[count] = Serial.read();
        count++;
    }
#ifdef debug
    Serial.println(message);
#endif
    // check message
}

/**
 * @brief read and decode the inverter request
 * 
 */
String decodeInverterRes()
{
    // skip 4 bytes
    const int offset = 6;
    String json = "{";
    json = json + "\"" + keys[0] + "\":" + String(get_16bit(offset + 0) * 0.1f) + ",";
    json = json + "\"" + keys[1] + "\":" + String(get_16bit(offset + 2) * 0.1f) + ",";
    json = json + "\"" + keys[2] + "\":" + String(get_16bit(offset + 4)) + ",";
    json = json + "\"" + keys[3] + "\":" + String(get_16bit(offset + 6) * 0.1f) + ",";
    json = json + "\"" + keys[4] + "\":" + String(get_16bit(offset + 8) * 0.1f) + ",";
    json = json + "\"" + keys[5] + "\":" + String(get_16bit(offset + 10) * 0.1f) + ",";
    json = json + "\"" + keys[6] + "\":" + String(get_16bit(offset + 12) * 0.1f) + ",";
    json = json + "\"" + keys[7] + "\":" + String(get_16bit(offset + 14)) + ",";
    json = json + "\"" + keys[8] + "\":" + String(get_16bit(offset + 16)) + ",";
    json = json + "\"" + keys[9] + "\":" + String(get_16bit(offset + 18) * 0.01f) + ",";
    json = json + "\"" + keys[10] + "\":" + String(get_16bit(offset + 20)) + ",";
    json = json + "\"" + keys[11] + "\":" + String(get_32bit(offset + 22) * 0.1f) + ",";
    json = json + "\"" + keys[12] + "\":" + String(get_16bit(offset + 26) * 0.1f) + ",";
    json = json + "\"" + keys[13] + "\":" + String(get_16bit(offset + 78) * 0.1f) + ",";
    json = json + "\"" + keys[14] + "\":" + String(get_16bit(offset + 82)) + "}";

    return json;
}

/**
 * @brief Get the 16bit object
 * 
 * @param i 
 * @return uint16_t 
 */
uint16_t get_16bit(size_t i)
{
    return (uint16_t(message[i + 1]) << 8) | (uint16_t(message[i]) << 0);
};

/**
 * @brief Get the 32bit object
 * 
 * @param i 
 * @return uint32_t 
 */
uint32_t get_32bit(size_t i)
{
    return uint32_t((message[i + 3] << 24) | (message[i + 2] << 16) | (message[i + 1] << 8) | message[i]);
};