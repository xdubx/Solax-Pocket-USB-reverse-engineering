#ifndef _MQTT_H
#define _MQTT_H
#include "Config.h"

#if MQTT_SUPPORTED == 1
#include <Arduino.h>
#include <PubSubClient.h>
// todo update to esp32
#include <ESP8266HTTPClient.h>

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
    void publish(const String& JsonString);
    void loop();

 private:
    WiFiClient& wifiClient;
    long previousConnectTryMillis = 0;
    MqttConfig config;
    PubSubClient client;
}
#endif
#endif
