#ifndef _MQTT_H
#define _MQTT_H
#include "Config.h"
#include <PubSubClient.h>




  #if MQTT_SUPPORTED == 1
    #include <Arduino.h>
    #include <WiFiClient.h>
    #include <PubSubClient.h>
    #include "WebDebug.h"
    
    typedef struct {
      String server;
      String port;
      String topic;
      String user;
      String pwd;
    } MqttConfig;

    class Mqtt{
    public:
        Mqtt(WiFiClient& wc) : wifiClient(wc), client(wifiClient){};
        void setup(const MqttConfig& config);
        bool reconnect();
        void publish(const String& json);
        void loop();

    private:
        WiFiClient& wifiClient;
        long previousConnectTryMillis = 0;
        int retryCounter = 0;
        MqttConfig config;
        PubSubClient client;
        static String getId();
    };

  #endif
#endif
