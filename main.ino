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
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "Arduino.h"

/**
 * @brief set the workmode of the esp
 *  api endpoint over http server
 *  send the values as json over http post
 */

//#define workModeAPI
#define workModeSEND
#define debug

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
    "runTime",
    "timestamp"};

const unsigned char usbRegister[] = {0xaa, 0x55, 0x11, 0x02, 0x01, 0x53, 0x57, 0x41, 0x4D, 0x54, 0x4C, 0x59, 0x34, 0x5A, 0x4D, 0x1f, 0x04};
const unsigned char requestSerial[] = {0xaa, 0x55, 0x11, 0x02, 0x01, 0x53, 0x57, 0x41, 0x4D, 0x54, 0x4C, 0x59, 0x34, 0x5A, 0x4D, 0x1f, 0x04};
const unsigned char requestData[] = {0xaa, 0x55, 0x07, 0x01, 0x05, 0x0c, 0x01};
// END requests

//GLOBAL VARS
unsigned char message[406];
int count = 0;
const char *ssid = "your wifi here";
const char *password = "your wifi pw here";
const char *host = "url here"; // example http://192.168.1.1:8085/hello

//END GLOBAL VARS

void setup()
{
    Serial1.begin(9600);
    Serial1.set_tx(15);
#ifdef debug
    Serial.begin(9600);

    bool wifi = createConnectionToWifi();
    if (wifi)
    {
#endif
#ifdef workModeAPI
        // Start the mDNS responder for solax.local
        if (MDNS.begin("solax"))
        {
#ifdef debug
            Serial.println("mDNS responder started");
#endif
        }
        else
        {
#ifdef debug
            Serial.println("Error setting up MDNS responder!");
#endif
        }

        server.on("/", handleRoot);
        server.begin(); // Actually start the server
#ifdef debug
        Serial.println("HTTP server started");
#endif
#endif

#ifdef workModeSEND

        // register dongle
        // pull data
        registerDongle();
        memset(message, 0, sizeof(message));

        delay(100);
        requestInverterData();
        sendRequest();
        //     // send to endpoint
        ESP.deepSleep(216000000000); // for ~1h
#endif
    }
    else
    {
        // go sleep or make something that the wifi is broke
        handleErrorLED(3, 1, 1);
        ESP.deepSleep(216000000000); // for ~1h
    }

    // flush dataholder
    memset(message, 0, sizeof(message));
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
#ifdef debug
        Serial.println("WLAN failed");
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
        WiFiClient client;
        HTTPClient http;
        String msg = decodeInverterRes();
        http.begin(client, host); // TODO: handle not reachable error
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(msg);
#ifdef debug
        Serial.println(httpCode);
#endif
        if (httpCode != 200)
        {
            handleErrorLED(2, 1, 2);
        }
    }
    else
    {
#ifdef debug
        Serial.println("Error in WiFi connection at runtime");
#endif
        // wifi get lost at runtime
        handleErrorLED(3, 1, 3);
    }
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
        Serial1.print(requestSerial[i]);
    }
    count = 0;

    while (Serial1.available() > 0 && count < 40)
    {
        message[count] = Serial1.read();
        count++;
    }
    // parse response
#ifdef debug
    for (size_t i = 0; i < count; i++)
    {
        Serial.print(message[i]);
    }
    Serial.println("");
#endif
    // calc LSB Checksum
    uint16_t checkSum = calcCheckSum(message, 44);
    uint16_t check = get_16bit(45);
    return checkSum == check;
}

/**
 * @brief register the usb dongle on the inverter
 * 
 */
void registerDongle()
{
    for (size_t i = 0; i < sizeof(usbRegister); i++)
    {
        Serial1.print(usbRegister[i]);
    }
    count = 0;
    while (Serial1.available() > 0 && count < 10)
    {
        message[count] = Serial1.read();
        count++;
    }
#ifdef debug
    for (size_t i = 0; i < count; i++)
    {
        Serial.print(message[i]);
    }
    Serial.println("");
#endif
    // has no checksum look readme.md
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
        Serial1.print(requestData[i]);
    }
    count = 0;
    while (Serial1.available() > 0 && count < 200)
    {
        message[count] = Serial1.read();
        count++;
    }
#ifdef debug
    for (size_t i = 0; i < count; i++)
    {
        Serial.print(message[i]);
    }
    Serial.println("");
#endif

    // calc LSB Checksum
    uint16_t checkSum = calcCheckSum(message, 204);
    uint16_t check = get_16bit(205);
    return checkSum == check;
}

/**
 * @brief read and decode the inverter request
 * 
 */
String decodeInverterRes()
{
    // skip 6 bytes
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
        server.send(200, "application/json", String("{\"error\": \"Failed to request data from inverter\""
    });
}
}
#endif

/**
 * @brief create a user feedback by the build in LED
 * 
 * @param firstBlink 
 * @param secBlind 
 * @param thirdBlink 
 */
void handleErrorLED(int firstBlink, int secBlind, int thirdBlink)
{
    for (int index = 0; index < 3; index++)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000 * firstBlink);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000 * secBlind);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000 * thirdBlink);
        digitalWrite(LED_BUILTIN, LOW);
        delay(5000);
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