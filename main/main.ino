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
#include "Arduino.h"

/**
 * @brief set the workmode of the esp
 *  api endpoint over http server
 *  send the values as json over http post
 */

//#define workModeAPI
#define workModeSEND

/**
 * @brief only one of both modes are posible if you activate both the controller crashed Serial1 and buildin led are on pin 2
 * 
 */
//#define debugSERIAL
//#define debugLED

#ifdef workModeSEND
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#endif

#ifdef workModeAPI
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
// create webserver
ESP8266WebServer server(80);
//protoype function
void handleRoot();
#endif

// requests
const String keys[] = {
    "gridVoltage",
    "gridCurrent",
    "gridPower",
    "pv1Voltage",
    "pv2Voltage",
    "pv1Current",
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
const unsigned char requestSerial[] = {0xaa, 0x55, 0x07, 0x01, 0x05, 0x0c, 0x01}; // TODO: optimise this function onto a facory with 2 parameter
const unsigned char requestData[] = {0xaa, 0x55, 0x07, 0x01, 0x0C, 0x13, 0x01};
const unsigned char requestSettings[] = {0xaa, 0x55, 0x07, 0x01, 0x16, 0x1D, 0x01};
// END requests

//GLOBAL VARS
unsigned char message[206];
const char *ssid = "your ssid";
const char *password = "your pw";
const char *host = "your destination"; // example http://raspberrypi:3001/data
#define timeout 5000
//END GLOBAL VARS

void setup()
{
    // delay 10 sec if the inverter has not enouth power and rebooting 
    delay(10000);
#ifdef debugLED
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
#endif
    Serial.begin(9600);
    //Serial.set_tx(1);
#ifdef debugSERIAL
    Serial1.begin(9600);
#endif
    //clear buffer
    memset(message, 0, sizeof(message));
    bool wifi = createConnectionToWifi();
    if (wifi)
    {

#ifdef workModeAPI
        // Start the mDNS responder for solax.local
        if (MDNS.begin("solax"))
        {
#ifdef debugSERIAL
            Serial1.println("mDNS responder started");
#endif
        }
        else
        {
#ifdef debugSERIAL
            Serial1.println("Error setting up MDNS responder!");
#endif
        }

        server.on("/", handleRoot);
        //TODO: add response for settings
        server.begin(); // Actually start the server
#ifdef debugSERIAL
        Serial1.println("HTTP server started");
#endif
#endif

#ifdef workModeSEND

        // register dongle
        // pull data
        bool reg = registerDongle();
        delay(50);
        if (reg)
        {
            memset(message, 0, sizeof(message));
            bool request = requestInverterData();
            if (!request)
            {
#ifdef debugLED
                handleErrorLED(2);
#endif
            }
            else
            {
                sendRequest();
            }
        }
        else
        {
#ifdef debugLED
            handleErrorLED(1);
#endif
        }
#endif
    }
    else
    {
// go sleep and wifi is broke
#ifdef debugLED
        handleErrorLED(3);
#endif
    }
#ifdef debugSERIAL
    Serial1.print("Go to deep sleep");
#endif
    WiFi.disconnect();
    ESP.deepSleep(36e8); // for ~1h
    delay(10);
}

void loop()
{
#ifdef workModeAPI
    //loop webserver
    server.handleClient();
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

#ifdef debugSERIAL
    Serial1.println("-------------------");
    Serial1.print("Connecting to ");
    Serial1.println(ssid);
#endif
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    byte tCounter = 0;
    while (tCounter < 60)
    {
        delay(1000);
#ifdef debugSERIAL
        Serial1.println(WiFi.status());
#endif
        if (WiFi.status() == WL_CONNECTED)
        {
#ifdef debugSERIAL
            Serial1.println("");
            Serial1.println("WiFi connected: ");
            Serial1.print(WiFi.localIP());
            Serial1.println();
#endif
            return true;
        }
        tCounter++;
    }
    if (tCounter >= 60)
    {
#ifdef debugSERIAL
        Serial1.println("WLAN failed");
#endif
        return false;
    }
}

/**
 * @brief send a request to the given host
 * 
 */
void sendRequest()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient wifiClient;
        HTTPClient http;
        String msg = decodeInverterRes();

        http.begin(wifiClient, host); // TODO: handle not reachable error
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(msg);
        String payload = http.getString();

        http.end();
        // do not set serial over or in http request the esp is crashing with exeption 9

        if (httpCode != HTTP_CODE_OK)
        {
#ifdef debugLED
            handleErrorLED(4);
#endif
#ifdef debugSERIAL
        Serial1.println("Http Code: ");
        Serial1.println(httpCode);
#endif
        }
    }
    else
    {
#ifdef debugSERIAL
        Serial1.println("Error in WiFi connection at runtime");
#endif
        // wifi get lost at runtime
#ifdef debugLED

        handleErrorLED(3);
#endif
    }
#ifdef debugSERIAL
    Serial1.println("End Function");
#endif
}

// END

/**
 * @brief Get the Inverter Serial object and write it into the message block
 * 
 * @return true if the checksum are correct
 * @return false if not
 */
bool getInverterSerial()
{
    // request serial of inverter
    for (size_t i = 0; i < sizeof(requestSerial); i++)
    {
        Serial.write(requestSerial[i]);
    }

    int counter = 0;
    long lastTime = millis();
    while (counter < 47)
    {
        if (Serial.available() > 0)
        {
            message[counter] = Serial.read();
            counter++;
        }
        else
        {
            if (millis() - lastTime > timeout)
            {
                break;
            }
        }
    }
    // parse response
#ifdef debugSERIAL
    for (size_t i = 0; i < counter; i++)
    {
        Serial1.print(message[i]);
    }
#endif
    // calc LSB Checksum
    uint16_t checkSum = calcCheckSum(message, 45);
    uint16_t check = get_16bit(46);
    return checkSum == check;
}

/**
 * @brief register the usb dongle on the inverter
 * 
 */
bool registerDongle()
{
    for (size_t i = 0; i < sizeof(usbRegister); i++)
    {
        Serial.write(usbRegister[i]);
    }
    int counter = 0;
    long lastTime = millis();
    while (counter < 14)
    {
        if (Serial.available() > 0)
        {
            message[counter] = Serial.read();
            counter++;
        }
        else
        {
            if (millis() - lastTime > timeout)
            {
                break;
            }
        }
    }

#ifdef debugSERIAL
    for (size_t i = 0; i < 14; i++)
    {
        Serial1.write(message[i]);
    }
    Serial1.println("");
#endif
    // has no checksum look readme.md
    // compare it with answer
    bool check = true;
    for (size_t index = 0; index < 14; index++)
    {
        if (usbRegister[index] != message[index])
        {
            check = false;
        }
    }

    return check;
}

/**
 * @brief request data at the inverter
 * 
 * @return true  if the checksum is correct
 * @return false if the checksum not match
 */
bool requestInverterData()
{
    for (size_t i = 0; i < sizeof(requestData); i++)
    {
        Serial.write(requestData[i]);
    }
    int counter = 0;
    long lastTime = millis();
    bool start = false;
    while (counter < 207)
    {
        if (Serial.available() > 0)
        {
            u_int8_t in = Serial.read();
            if (start)
            {
                message[counter] = in;
                counter++;
            }
            else
            {
                // filter the spam before the message in this case it is a b'M'
                if (in == 0xaa)
                {
                    start = true;
                    message[counter] = in;
                    counter++;
                }
            }
        }
        else
        {
            if (millis() - lastTime > timeout)
            {
                break;
            }
        }
    }

#ifdef debugSERIAL
    for (size_t i = 0; i < counter; i++)
    {
        Serial1.write(message[i]);
    }
    Serial1.println("");
#endif

    // calc LSB Checksum
    uint16_t checkSum = calcCheckSum(message, 204);
    uint16_t check = get_16bit(205);
    if (checkSum == 0x00)
        return false;
    return checkSum == check;
}

/** TODO: create for this a parser and increase message size to 407 bytes
 * @brief request the inverter settings
 * @return true
 * @return false
 */
bool requestInverterSettings()
{
    for (size_t i = 0; i < sizeof(requestSettings); i++)
    {
        Serial.write(requestSettings[i]);
    }

    int counter = 0;
    long lastTime = millis();
    while (counter < 407)
    {
        if (Serial.available() > 0)
        {
            message[counter] = Serial.read();
            counter++;
        }
        else
        {
            if (millis() - lastTime > timeout)
            {
                break;
            }
        }
    }

#ifdef debugSERIAL
    for (size_t i = 0; i < counter; i++)
    {
        Serial1.print(message[i], HEX);
    }
    Serial1.println("");
#endif

    // calc LSB Checksum
    uint16_t checkSum = calcCheckSum(message, 404);
    uint16_t check = get_16bit(405);
    return checkSum == check;
}

/**
 * @brief read and decode the inverter request
 * 
 */
String decodeInverterRes()
{
    // skip 5 bytes
    const int offset = 5;
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
    json = json + "\"" + keys[13] + "\":" + String(get_16bit(offset + 78)) + ",";
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

#ifdef workmodeAPI
/**
 * weberver section
 */

void handleRoot()
{
    bool success = requestInverterData();
    // request data
    if (success)
    {
        server.send(200, "application/json", decodeInverterRes());
    }
    else
    {
        server.send(200, "application/json", String("{\"error\": \"Failed to request data from inverter\"}")
    });
}
}
#endif

/**
 * @brief create a user feedback by the build in LED
 * 
 * @param blinkTimes 
 */
void handleErrorLED(int blinkTimes)
{

    for (int j = 0; j < 3; j++)
    {
        for (int index = 0; index < blinkTimes; index++)
        {
            digitalWrite(2, LOW);
            delay(800);
            digitalWrite(2, HIGH);
            delay(1000);
        }
        delay(3000);
    }
}

/**
 * @brief calc the checksum of the message block
 * 
 * @param data 
 * @param len 
 * @return uint16_t 
 */
uint16_t calcCheckSum(const uint8_t data[], const uint8_t len)
{
    uint16_t checksum = 0;
    for (uint8_t index = 0; index <= len; index++)
    {
        checksum = checksum + data[index];
    }
    return checksum;
}
