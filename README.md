# What is this?
Custom Firmware for a Solax WIFI USB Stick as an esp32/8266

[![Build with PlatformIO](https://img.shields.io/badge/PlatformIO-Build-orange)](https://platformio.org/)

[![ESP8266](https://img.shields.io/badge/ESP-8266-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp8266) [![ESP32](https://img.shields.io/badge/ESP-32-000000.svg?longCache=true&style=flat&colorA=CC101F)](https://www.espressif.com/en/products/socs/esp32)

**This works with the ESP8266 Arduino platform**

[https://github.com/esp8266/Arduino](https://github.com/esp8266/Arduino)

**This works with the ESP32 Arduino platform** 

[https://github.com/espressif/arduino-esp32](https://github.com/espressif/arduino-esp32)


# ESP8266

## Pinout of ESP
|ESP|USB|
|---|---|
|GND (USB)|GND|       
|VIN (USB)|5V|       
|GPI01 (TX)(USB)|TX|
|GPI03 (RX)(USB)|RX|


|ESP|DEBUG USB|
|---|---|
|GPI02 | RX |  

TODO: add image 

## Features
Implemented Features:
* Built-in simple Webserver
* Simple Webhook
* The data received will be transmitted by MQTT to a server of your choice.
* The data received is also provied as JSON
* Wifi manager with own access point for initial configuration of Wifi and MQTT server 
* TLS support for esp32

## Supported microcontrollers
* NODEMCU V1 (ESP8266) with a Solax Inverter connected via USB (USB-Serial Chip: CH340)

## Supported Inverters
* X1 Mini

## Structure
### JSON
For ENDPOINT Data and sended post request
```json
{
    "gridVoltage": number,
    "gridCurrent": number,
    "gridPower": number,
    "pv1Voltage": number,
    "pv1Current": number,
    "pv2Voltage": number,
    "pv2Current": number,
    "pv1Power": number,
    "pv2Power": number,
    "gridFrequency": number,
    "mode": number,
    "eTotal": number,
    "eToday": number,
    "temp": number,
    "runTime": number,
}

```

## Code 
Still missing freatures are: 
- Parse Serial
- Parse Settings
- pwr limit
  
Notice: Only the register dongle and request data is tested


# Build of request
Spezial thanks go to [@tuxmike](https://github.com/tuxmike)

## Message structure (header, payload, checksum)
|Byte offset|Datatype|Description|Value|
|---|---|---|---|
|0|uint16|Static preamble|0xAA 0x55|
|2|uint8|Total msg frame size (byte)|size of: header + payload + checksum
|3|uint8|Cmd control code|e.g. 0x01|
|4|uint8|Cmd function code|e.g. 0x8C|
|5|...|payload|see cmd table|
|[Frame size-2]|uint16|Checksum|uint16 binary sum of all msg bytes|


## Commands
|Control code|Function Code|Payload length (byte)|Checksum length (byte)|Description|
|---|---|---|---|---|
|01|05|0|2|Request read serial numbers|
|01|85|40|2|Response read serial numbers|
|02|01|10|2|Request register pocket dongle serial number|
|02|01|10|0 (!)|Response register pocket dongle serial number|
|01|16|0|2|Request inverter settings|
|01|96|400|2|Response inverter settings|
|01|0C|0|2|Request inverter data|
|01|8C|200|2|Response inverter data|
|01|04|0|2|Request inverter error data|
|01|84|44|2|Response inverter error data|
|05|00|2|2|Request enter pin|
|05|42|2|2|Request write setting pwr limit value|
|05|60|2|2|Request write setting pwr limit on/off|
|03|80|0|2|Response enter pin ok|
|03|81|0|2|Response setting write ok|

## Response serial numbers (Control code=0x01, FuncCode=0x85)
|Byte offset|Datatype|Description|
|---|---|---|
|0|char[14]|Inverter serial number|
|14|char[14]|Inverter serial number padding, spaces|
|28|uint16|Inverter model code|
|30|char[8]|Pocket dongle serial number|
|38|uint16|Inverter model type|

## Request / Response register pocket dongle serial number (Control code=0x02, FuncCode=0x01)
|Byte offset|Datatype|Description|
|---|---|---|
|0|char[10]|Pocket dongle serial number|

## Response inverter data (Control code=0x01, FuncCode=0x8C)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|Grid voltage|0.1V|
|2|uint16|Grid current|0.1A|
|4|uint16|Grid power|1W|
|6|uint16|PV1 voltage|0.1V|
|8|uint16|PV2 voltage|0.1A|
|10|uint16|PV1 current|0.1V|
|12|uint16|PV2 current|0.1A|
|14|uint16|PV1 power|1W|
|16|uint16|PV2 power|1W|
|18|uint16|Grid frequency|0.01Hz|
|20|uint16|Mode| See Invertermodes |
|22|uint32|E Total|0.1kwh|
|26|uint16|E Today|0.1kwh|
|28|?|?|?|
|...|...|...|...|
|78|uint16|Inverter Temperature|°C|
|82|uint32|Runtime-total|1h|
|86|?|?|?|
|...|...|...|...|
|110|uint8|0x1B ?|?|
|...|...|...|...|

## Inverter Modes
|Code|Description|
|---|---|
|0 |Waiting|
|1 |Checking|
|2 |Normal|
|3 |Fault|
|4 |Permanent Fault|
|5 |Update|
|6 |Off-grid waiting|
|7 |Off-grid|
|8 |Self Testing|
|9 |Idle|
|10|Standby|


## Response inverter error data (Control code=0x01, FuncCode=0x84)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|PV1 voltage|0.1V|
|2|uint16|PV2 voltage|0.1V|
|4|uint16|?|?|
|6|uint16|Grid voltage|0.1V|
|8|uint16|Grid frequency|0.01Hz|
|10|?|?|?|
|...|...|...|...|

## Response inverter settings (Control code=0x01, FuncCode=0x96)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|VAC low|0.1V|
|2|uint16|VAC high|0.1V|
|4|uint16|FAC low|0.01Hz|
|6|uint16|FAC high|0.01Hz|
|8|uint16|?|0.1?|
|12|uint16|VAC 10m avg |0.1V|
|14|uint16|VAC low slow |0.1V|
|16|uint16|VAC high slow |0.1V|
|18|uint16|FAC low slow|0.01Hz|
|20|uint16|FAC high slow|0.01Hz|
|...|...|...|...|
|208|uint16|Settings PIN A|decimal|
|250|uint16|Settings PIN B (?)|decimal|
|280|uint16|Start time|s|
|...|...|...|...|

## Request enter pin (Control code=0x05, FuncCode=0x00)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|PIN|decimal|

## Request write setting pwr limit value (Control code=0x05, FuncCode=0x42)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|power limit|W|

## Request write setting pwr limit on/off (Control code=0x05, FuncCode=0x60)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|power limit on/off|0U=off, 1U=on|

## Values that are missing
* Battery
If you have a battery connected to your inverter -> You can read the values out and print it in a issue.