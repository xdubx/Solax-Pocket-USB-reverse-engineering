#ifndef _SOLAX_H_
#define _SOLAX_H_

#include "Config.h"
#include "Arduino.h"

class Solax
{
public:
    Solax();
    bool registerDongle();
    bool getInverterSerial();
    bool requestInverterData();
    bool requestInverterSettings();
    uint16_t calcCheckSum(const uint8_t data[], const uint8_t len);
    uint16_t get_16bit(size_t i);
    uint32_t get_32bit(size_t i);
    String decodeInverterRes();
};

#endif