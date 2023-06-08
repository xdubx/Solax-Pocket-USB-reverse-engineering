#include "Mqtt.h"
#if MQTT_SUPPORTED == 1

void Mqtt::setup(const MqttConfig &config)
{
    this->config = config;

    uint16_t intPort = config.port.toInt();
    if (intPort == 0)
        intPort = 1883;
    this->client.setBufferSize(MQTT_MAX_PACKET_SIZE);
    this->client.setServer(this->config.server.c_str(), intPort);
}

bool Mqtt::reconnect()
{
    if (this->config.server.length() == 0)
    {
        // No server configured throw error
        return false;
    }
    // if (WiFi.status() != WL_CONNECTED) return false;
    if (this->client.connected()){
        return true;
    }

    if(this->retryCounter > 5){
        return false;
    }

    if ((millis() - this->previousConnectTryMillis) >= 5000)
    {
        this->previousConnectTryMillis = millis();
        if(this->client.connect(getId().c_str(),
                                 this->config.user.c_str(),
                                 this->config.pwd.c_str(),
                                 this->config.topic.c_str(), 1, 1,
                                 "{\"InverterStatus\": -1 }")){
            this->retryCounter = 0;
            return true;
        }else{
            this->retryCounter++;
            WEB_DEBUG_PRINT("MQTT Connect failed");
        }
    }
    return false;
}

void Mqtt::publish(const String &json)
{
    if (this->client.connected())
    {
        this->client.publish(this->config.topic.c_str(), json.c_str(), true);
    }
    else
    {
         WEB_DEBUG_PRINT("MQTT Publish failed");
    }
}

void Mqtt::loop()
{
    this->client.loop();
}


String Mqtt::getId() {
#ifdef ESP8266
  uint64_t id = ESP.getChipId();
#elif ESP32
  uint64_t id = ESP.getEfuseMac();
#endif
  return "Solax" + String(id & 0xffffffff);
}
#endif
