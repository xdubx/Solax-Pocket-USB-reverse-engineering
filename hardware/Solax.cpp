#include "Solax.h"

const unsigned char usbRegister[] = {0xaa, 0x55, 0x11, 0x02, 0x01, 0x53, 0x57, 0x41, 0x4D, 0x54, 0x4C, 0x59, 0x34, 0x5A, 0x4D, 0x1f, 0x04};
const unsigned char requestSerial[] = {0xaa, 0x55, 0x07, 0x01, 0x05, 0x0c, 0x01}; // TODO: optimise this function onto a facory with 2 parameter
const unsigned char requestData[] = {0xaa, 0x55, 0x07, 0x01, 0x0C, 0x13, 0x01};
const unsigned char requestSettings[] = {0xaa, 0x55, 0x07, 0x01, 0x16, 0x1D, 0x01};

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

unsigned char message[206];


Solax::Solax(){
        //clear buffer
    memset(message, 0, sizeof(message));
}

/**
 * @brief register the usb dongle on the inverter
 * 
 */
bool Solax::registerDongle()
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
            if (millis() - lastTime > inverter_timeout)
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
    // clear buffer
    memset(message, 0, sizeof(message));
    return check;
}

/**
 * @brief Get the Inverter Serial object and write it into the message block
 * 
 * @return true if the checksum are correct
 * @return false if not
 */
bool Solax::getInverterSerial()
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
            if (millis() - lastTime > inverter_timeout)
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
 * @brief request data at the inverter
 * 
 * @return true  if the checksum is correct
 * @return false if the checksum not match
 */
bool Solax::requestInverterData()
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
            if (millis() - lastTime > inverter_timeout)
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
bool Solax::requestInverterSettings()
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
            if (millis() - lastTime > inverter_timeout)
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
 * @brief calc the checksum of the message block
 * 
 * @param data 
 * @param len 
 * @return uint16_t 
 */
uint16_t Solax::calcCheckSum(const uint8_t data[], const uint8_t len)
{
    uint16_t checksum = 0;
    for (uint8_t index = 0; index <= len; index++)
    {
        checksum = checksum + data[index];
    }
    return checksum;
}



/**
 * @brief Get the 16bit object
 * 
 * @param i 
 * @return uint16_t 
 */
uint16_t Solax::get_16bit(size_t i)
{
    return (uint16_t(message[i + 1]) << 8) | (uint16_t(message[i]) << 0);
};

/**
 * @brief Get the 32bit object
 * 
 * @param i 
 * @return uint32_t 
 */
uint32_t Solax::get_32bit(size_t i)
{
    return uint32_t((message[i + 3] << 24) | (message[i + 2] << 16) | (message[i + 1] << 8) | message[i]);
};


/**
 * @brief read and decode the inverter request
 * 
 */
String Solax::decodeInverterRes()
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
