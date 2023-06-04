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


#if MQTT_SUPPORTED == 1
    #include "Mqtt.h"
    void loadMqttConfig(MqttConfig* config);

#endif


#if WEB_REQUEST_SUPPORTED == 1
    #include <WiFiClient.h>
    #include <ESP8266HTTPClient.h>

    typedef struct {
        String server;
        String port;
    } WebConfig;

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

#if MQTT_SUPPORTED == 1
    WiFiManagerParameter* custom_mqtt_server = NULL;
    WiFiManagerParameter* custom_mqtt_port = NULL;
    WiFiManagerParameter* custom_mqtt_topic = NULL;
    WiFiManagerParameter* custom_mqtt_user = NULL;
    WiFiManagerParameter* custom_mqtt_pwd = NULL;

    const static char* serverfile = "/mqtts";
    const static char* portfile = "/mqttp";
    const static char* topicfile = "/mqttt";
    const static char* userfile = "/mqttu";
    const static char* secretfile = "/mqttw";
#endif

#if WEB_REQUEST_SUPPORTED == 1
    WiFiManagerParameter* custom_web_server = NULL;
    WiFiManagerParameter* custom_web_port = NULL;

    const static char* webServerFile = "/webserver";
    const static char* webPortFile = "/webport";
#endif



void setup()
{
    // delay 10 sec if the inverter has not enouth power and rebooting 
    // don't send empty requests
    delay(10000);
    #ifdef debugLED
        pinMode(2, OUTPUT);
        digitalWrite(2, HIGH);
    #endif
    
    Serial.begin(9600);

    #ifdef debugSERIAL
        Serial1.begin(9600);
    #endif

    WiFi.hostname(HOSTNAME); // TODO: check if mdns here is needed? 

    // Setup the wifimanager
    // Set a timeout so the ESP doesn't hang waiting to be configured, for instance after a power failure
    wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);
    // Automatically connect using saved credentials -> if connection failes the wifi manager start as ap
    //TODO: load configs


    setupWifiManagerMenu();

    bool wifi = wifiManager.autoConnect(HOSTNAME, APPassword);

    if (!wifi)
    {
        // no connection to wifi posible
        // TODO: print debug 
        #ifdef debugLED
            handleErrorLED(3);
        #endif
        // restart after the timeout for the wifimanager
        ESP.restart();
    }
    else
    {
        // here wifi is connected successfull to router
        // now we can work with the device
    }


    // TODO: overwork this section 
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
                        // replace host from config
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
 * @brief create custom wifimanager menu entries
 * 
 * @param enableCustomParams enable custom params aka. mqtt settings
 */
// TODO: put webconf
void setupWifiManagerMenu(){
    // check if webrequest is enabled
    bool enableCustomParams = false;

    #if WEB_REQUEST_SUPPORTED == 1
        enableCustomParams = true;
        // TODO: update to webconfig
        loadWebConfig(&webConfig);

        custom_web_server = new WiFiManagerParameter("server", "web server", webConfig.server.c_str(), 40);
        custom_web_port = new WiFiManagerParameter("port", "web port", webConfig.port.c_str(), 6);

        wifiManager.addParameter(custom_web_server);
        wifiManager.addParameter(custom_web_port);

        // REPLACE FUNCTION
        wifiManager.setSaveConfigCallback(saveParamCallback);

        // save callback
    #endif

    // check if mqtt is enabled
    #if MQTT_SUPPORTED == 1
        enableCustomParams = true;
        loadMqttConfig(&mqttConfig);

        custom_mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqttConfig.server.c_str(), 40);
        custom_mqtt_port = new WiFiManagerParameter("port", "mqtt port", mqttConfig.port.c_str(), 6);
        custom_mqtt_topic = new WiFiManagerParameter("topic", "mqtt topic", mqttConfig.topic.c_str(), 64);
        custom_mqtt_user = new WiFiManagerParameter("username", "mqtt username", mqttConfig.user.c_str(), 40);
        custom_mqtt_pwd = new WiFiManagerParameter("password", "mqtt password", mqttConfig.pwd.c_str(), 64);

        wifiManager.addParameter(custom_mqtt_server);
        wifiManager.addParameter(custom_mqtt_port);
        wifiManager.addParameter(custom_mqtt_topic);
        wifiManager.addParameter(custom_mqtt_user);
        wifiManager.addParameter(custom_mqtt_pwd);
        
        wifiManager.setSaveConfigCallback(saveParamCallback);
    #endif
}

#if WEB_REQUEST_SUPPORTED == 1

void loadWebConfig(WebConfig* config)
{
    config->server = prefs.getString(serverfile, "192.168.178.2");
    config->port = prefs.getString(portfile, "80");
}

#endif


// ------ MQTT helper functions

#if MQTT_SUPPORTED == 1

/**
 * @brief load config from prefs
 * 
 * @param config 
 */
void loadMqttConfig(MqttConfig* config)
{
    config->server = prefs.getString(serverfile, "10.1.2.3");
    config->port = prefs.getString(portfile, "1883");
    config->topic = prefs.getString(topicfile, "energy/solar");
    config->user = prefs.getString(userfile, "");
    config->pwd = prefs.getString(secretfile, "");
}

/**
 * @brief save the current mqttconfig
 * 
 * @param config 
 */
void saveMqttConfig(MqttConfig* config)
{
    prefs.putString(serverfile, config->server);
    prefs.putString(portfile, config->port);
    prefs.putString(topicfile, config->topic);
    prefs.putString(userfile, config->user);
    prefs.putString(secretfile, config->pwd);
}

/**
 * @brief save callback for the wifimanager to save the mqtt stuff
 * 
 */
void saveParamCallback()
{
    MqttConfig config;

    config.server = custom_mqtt_server->getValue();
    config.port = custom_mqtt_port->getValue();
    config.topic = custom_mqtt_topic->getValue();
    config.user = custom_mqtt_user->getValue();
    config.pwd = custom_mqtt_pwd->getValue();

    saveMqttConfig(&config);

    //ESP.restart();
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


        // #ifdef workModeAPI
        // // Start the mDNS responder for solax.local
        //     if (MDNS.begin("solax"))
        //     {
        //         #ifdef debugSERIAL
        //             Serial1.println("mDNS responder started");
        //         #endif
        //     }
        //     else
        //     {
        //         #ifdef debugSERIAL
        //             Serial1.println("Error setting up MDNS responder!");
        //         #endif
        //     }
        //     server.on("/", handleRoot);
        //     server.begin(); // start the server

        //     #ifdef debugSERIAL
        //             Serial1.println("HTTP server started");
        //     #endif
        // #endif

        // #ifdef WEB_REQUEST_SUPPORTED
        //     // register dongle
        //     // pull data
        //     bool reg = Inverter.registerDongle();
        //     delay(50);
        //     if (reg)
        //     {
        //         bool request = Inverter.requestInverterData();
        //         if (!request)
        //         {
        //             #ifdef debugLED
        //                 handleErrorLED(2);
        //             #endif
        //         }
        //         else
        //         {
        //             sendRequest();
        //         }
        //     }
        //     else
        //     {
        //         #ifdef debugLED
        //             handleErrorLED(1);
        //         #endif
        //     }
        // #endif