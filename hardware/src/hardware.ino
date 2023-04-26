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
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "Config.h"
#include "Solax.h"


#ifdef workModeSEND
    #include <WiFiClient.h>
    #include <ESP8266HTTPClient.h>
#endif

#ifdef workModeAPI
    #include <ESP8266WebServer.h>
    // create webserver
    ESP8266WebServer server(80);
    //protoype function
    void handleRoot();
#endif

#define CONFIG_PORTAL_MAX_TIME_SECONDS 300
Solax Inverter;
WiFiManager wifiManager;

// requests

void setup()
{
    // delay 10 sec if the inverter has not enouth power and rebooting 
    // don't send small or empty requests
    delay(10000);
    #ifdef debugLED
        pinMode(2, OUTPUT);
        digitalWrite(2, HIGH);
    #endif
    Serial.begin(9600);

    #ifdef debugSERIAL
        Serial1.begin(9600);
    #endif

    WiFi.hostname(HOSTNAME);

    wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);
    bool wifi = wifiManager.autoConnect(HOSTNAME, APPassword);

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
            server.begin(); // start the server

            #ifdef debugSERIAL
                    Serial1.println("HTTP server started");
            #endif
        #endif

        #ifdef workModeSEND
            // register dongle
            // pull data
            bool reg = Inverter.registerDongle();
            delay(50);
            if (reg)
            {
                bool request = Inverter.requestInverterData();
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
 * @brief send a request to the given host
 * 
 */
void sendRequest()
{

    // TODO: handle here mqtt class
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient wifiClient;
        HTTPClient http;
        String msg = Inverter.decodeInverterRes();

        http.begin(wifiClient, "test.de"); // TODO: handle not reachable error
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
        // wifi get lost at runtime
        #ifdef debugSERIAL
                Serial1.println("Error in WiFi connection at runtime");
        #endif

        #ifdef debugLED
                handleErrorLED(3);
        #endif
    }
}

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
        server.send(200, "application/json", String("{\"error\": \"Failed to request data from inverter\"}"));
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
