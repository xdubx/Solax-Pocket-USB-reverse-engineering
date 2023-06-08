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
#include <WiFiManager.h>
#include <Preferences.h>
#include "Config.h"
#include "Solax.h"
#include "WebDebug.h"

// BOARD selection


#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266mDNS.h>
#elif ESP32
    #ifdef MQTTS_BROKER_CA_CERT
        #define MQTTS_ENABLED 1
    #endif

#ifdef MQTTS_ENABLED
    #include <WiFiClientSecure.h>
#else
    #include <WiFiClient.h>
#endif
    #include <HTTPClient.h>
    #include <ESPmDNS.h>
    #include <WebServer.h>
#endif


#if MQTT_SUPPORTED == 1
    #include "Mqtt.h"
    void loadMqttConfig(MqttConfig* config);
    void saveMqttConfig(MqttConfig* config);
    
    #ifdef MQTTS_ENABLED
        WiFiClientSecure espClient;
    #else
        WiFiClient espClient;
    #endif

    Mqtt mClient(espClient);

#endif


#if WEB_REQUEST_SUPPORTED == 1
    #include <WiFiClient.h>

    typedef struct {
        String server;
    } WebConfig;

    void loadWebConfig(WebConfig* config);
    void sendRequest(String msg);
    #if ENABLE_WEB_DEBUG == 1
        void handleDebug();
    #endif
#endif

#if WEBSERVER_SUPPORTED == 1
    #ifdef ESP8266
        ESP8266WebServer server(80);
    #elif ESP32
        WebServer server(80);
    #endif
    //protoype function
    void handleRoot();
#endif

#define CONFIG_PORTAL_MAX_TIME_SECONDS 300
Solax Inverter;
WebConfig webConfig;
MqttConfig mqttConfig;

WiFiManager wifiManager;
Preferences prefs;

long RefreshTimer = 0;

//prototype function
void savePrefCallback();
void setupWifiManagerMenu(WebConfig webConfig, MqttConfig mqttConfig);

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

    const static char* webServerFile = "/webserver";
#endif



void setup()
{
    // delay 10 sec if the inverter has not enouth power and or rebooting 
    // don't send empty requests
    delay(10000); // set a flag for sun rising
    #ifdef debugLED
        pinMode(2, OUTPUT);
        digitalWrite(2, HIGH);
    #endif
    
    Serial.begin(9600);

    #ifdef ENABLE_DEBUG_OUTPUT
        Serial1.begin(9600);
    #endif

    WiFi.hostname(HOSTNAME); // TODO: check if mdns here is needed? 
    // Setup the wifimanager
    // Set a timeout so the ESP doesn't hang waiting to be configured, for instance after a power failure
    wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);
    // Automatically connect using saved credentials -> if connection failes the wifi manager start as ap

    prefs.begin("Solax");

    #if WEB_REQUEST_SUPPORTED == 1
        loadWebConfig(&webConfig);
    #endif
    #if MQTT_SUPPORTED == 1
        loadMqttConfig(&mqttConfig);
    #endif

    setupWifiManagerMenu(webConfig, mqttConfig);

    bool wifi = wifiManager.autoConnect(HOSTNAME, APPassword);

    if (!wifi)
    {
        // no connection to wifi posible
        // TODO: print debug 
        #ifdef debugLED
            handleErrorLED(3);
        #endif
          WEB_DEBUG_PRINT("WIFI connect timeout");
        // restart after the timeout for the wifimanager
        ESP.restart();
    }
    else
    {
        // here wifi is connected successfull to router
        // now we can work with the device
    }


    bool success = Inverter.registerDongle();
    if(success){

        #if WEBSERVER_SUPPORTED == 1
            if (MDNS.begin("solax"))
            {
                #ifdef ENABLE_DEBUG_OUTPUT
                    Serial1.println("mDNS responder started");
                #endif
            }
            else
            {
                #ifdef ENABLE_DEBUG_OUTPUT
                    Serial1.println("Error setting up MDNS responder!");
                #endif
            }
            
            server.on("/", handleRoot);
            
            #if ENABLE_WEB_DEBUG == 1
                server.on("/debug", handleDebug);
            #endif

            server.begin();
            #ifdef ENABLE_DEBUG_OUTPUT
                Serial1.println("HTTP server started");
            #endif
            
        #endif

        #if MQTT_SUPPORTED == 1
            WEB_DEBUG_PRINT("mClient.setup");
            mClient.setup(mqttConfig);
        #endif
    }else{
        WEB_DEBUG_PRINT("Failed to register dongle");
        #ifdef ENABLE_DEBUG_OUTPUT
            Serial1.println("Failed to register dongle");
        #endif
    }
}

void loop()
{

    long now = millis();

    #if WEBSERVER_SUPPORTED == 1
        //loop webserver
        server.handleClient();
    #endif

    // handle mqtt
    #if MQTT_SUPPORTED == 1
        mClient.loop();
    #endif

    // check wifi connection? 
    Wifi_Reconnect();

    if ((now - RefreshTimer) > REFRESH_TIMER){
     
        Inverter.requestInverterData();

        // create json
        WEB_DEBUG_PRINT("Request data");
        String message = Inverter.decodeInverterRes();

        #if WEB_REQUEST_SUPPORTED == 1
           sendRequest(message);
        #endif

        #if MQTT_SUPPORTED == 1
            WEB_DEBUG_PRINT("mClient.publish()");
            mClient.publish(message);
        #endif
    }
    delay(10);
}



#if WEB_REQUEST_SUPPORTED == 1
/**
 * @brief send a request to the given host
 * 
 */
void sendRequest(String msg)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient wifiClient;
        HTTPClient http;
        http.begin(wifiClient, webConfig.server); // TODO: handle not reachable error
        
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(msg);
        String payload = http.getString();

        http.end();
        // do not set serial over or in http request the esp is crashing with exeption 9

        if (httpCode != HTTP_CODE_OK)
        {
            String message = "HTTP Error: " + String(httpCode);
            WEB_DEBUG_PRINT(message.c_str());
            #ifdef debugLED
                handleErrorLED(4);
            #endif
            #ifdef ENABLE_DEBUG_OUTPUT
                Serial1.println("Http Code: ");
                Serial1.println(httpCode);
            #endif
            // TODO: DEBUG WEB
        }
    }
    else
    {
        // wifi get lost at runtime
        #ifdef ENABLE_DEBUG_OUTPUT
                Serial1.println("Error in WiFi connection at runtime");
        #endif

        #ifdef debugLED
                handleErrorLED(3);
        #endif
    }
}
#endif

#if WEBSERVER_SUPPORTED == 1
    /**
     * weberver section
     */

    void handleRoot()
    {
        bool success = Inverter.requestInverterData();
        // request data
        if (success)
        {
            server.send(200, "application/json", Inverter.decodeInverterRes());
        }
        else
        {
            server.send(200, "application/json", String("{\"error\": \"Failed to request data from inverter\"}"));
        }
    }

    #if ENABLE_WEB_DEBUG == 1

        void handleDebug()
        {
            server.send(200, "text/plain", acWebDebug);
        }
    #endif
#endif



/**
 * @brief create custom wifimanager menu entries
 * 
 * @param webConfig for the webrequest
 * @param mqttConfig for the mqtt
 */
void setupWifiManagerMenu(WebConfig webConfig, MqttConfig mqttConfig){


    #if WEB_REQUEST_SUPPORTED == 1
        custom_web_server = new WiFiManagerParameter("server", "web server", webConfig.server.c_str(), 64);

        wifiManager.addParameter(custom_web_server);
    #endif

    // check if mqtt is enabled
    #if MQTT_SUPPORTED == 1

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

    #endif
    wifiManager.setSaveConfigCallback(savePrefCallback);
}


// ------ WEB helper functions

#if WEB_REQUEST_SUPPORTED == 1

void loadWebConfig(WebConfig* config)
{
    config->server = prefs.getString(webServerFile, "http://192.168.178.2:80");
}

/**
 * @brief save the current webconfig
 * 
 * @param config 
 */
void saveWebConfig(WebConfig* config)
{
    prefs.putString(webServerFile, config->server);
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
    config->server = prefs.getString(serverfile, "192.168.178.2");
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

#endif

void savePrefCallback(){
    #if MQTT_SUPPORTED == 1
        MqttConfig mqttConfig;

        mqttConfig.server = custom_mqtt_server->getValue();
        mqttConfig.port = custom_mqtt_port->getValue();
        mqttConfig.topic = custom_mqtt_topic->getValue();
        mqttConfig.user = custom_mqtt_user->getValue();
        mqttConfig.pwd = custom_mqtt_pwd->getValue();

        saveMqttConfig(&mqttConfig);
    #endif

    #if WEB_REQUEST_SUPPORTED == 1
        WebConfig webConfig;

        webConfig.server = custom_web_server->getValue();

        saveWebConfig(&webConfig);
   #endif

}


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

void Wifi_Reconnect(){
    if (WiFi.status() != WL_CONNECTED)
    {
        wifiManager.autoConnect();

        int retryCounter = 0;

        while (WiFi.status() != WL_CONNECTED)
        {
            // add counter for breaking while loop
            delay(200);
            #if ENABLE_DEBUG_OUTPUT == 1
                Serial1.println("Failed to register dongle");
            #endif

            WEB_DEBUG_PRINT("Reconnecting");
        }

        WEB_DEBUG_PRINT("WiFi reconnected")
    }
}