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
        // No server configured
        return false;
    }
    // if (WiFi.status() != WL_CONNECTED) return false;
    if (this->client.connected())
        return true;

    this->previousConnectTryMillis = millis();

    if (millis() - this->previousConnectTryMillis >= (5000))
    {
    }
}

void Mqtt::publish(const String &json)
{
    if (this->client.connected())
    {
        this->client.publish(this->config.topic.c_str(), json.c_str(), true);
    }
    else
    {
        // TODO: throw error
    }
}

void Mqtt::loop()
{
    this->client.loop();
}
#endif
