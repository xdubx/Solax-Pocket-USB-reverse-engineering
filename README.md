# What is this?

I try to revese the solar stick at the communication level with the solar inverter from Solax.
In parallel, I'm looking at ghirda to maybe speed things up. I would be very grateful for any tips.

## Which µC this repo using 
For a fast development i wrote for the esp8622 in arduino cpp. Pinout below. 

# Build of request
Spezial thanks go to @tuxmike

## Message structure (header, payload, checksum)
|Byte offset|Datatype|Description|Value|
|---|---|---|---|
|0|uint16|Static preamble|0xAA55|
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
|01|16|0|2|Request ?|
|01|96|400|2|Response ?|
|01|0C|0|2|Request inverter data|
|01|8C|200|2|Response inverter data|
|01|04|0|2|Request inverter error data|
|01|84|44|2|Response inverter error data|

## Serial numbers (Control code=0x01, FuncCode=0x85)
|Byte offset|Datatype|Description|
|---|---|---|
|0|char[14]|Inverter serial number|
|14|char[14]|Inverter serial number padding, spaces|
|28|uint16|Inverter model code|
|30|char[8]|Pocket dongle serial number|
|38|uint16|Inverter model type|

## Register pocket dongle serial number (Control code=0x02, FuncCode=0x01)
|Byte offset|Datatype|Description|
|---|---|---|
|0|char[10]|Pocket dongle serial number|

## Inverter data (Control code=0x01, FuncCode=0x8C)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|Grid voltage|0.1V|
|2|uint16|Grid current|0.1A|
|4|uint16|Grid power|1W|
|6|uint16|PV1 voltage|0.1V|
|8|uint16|PV1 current|0.1A|
|10|uint16|PV2 voltage|0.1V|
|12|uint16|PV2 current|0.1A|
|14|uint16|PV1 power|1W|
|16|uint16|PV2 power|1W|
|18|uint16|Grid frequency|0.01Hz|
|20|uint16|Mode|0: Wait, 1: Check, 2: Normal/Running, 3: Fault, (4: PermanentFault, 5: UpdateMode, ...?)|
|22|uint32|E Total|0.1kwh|
|26|uint16|E Today|0.1kwh|
|28|?|?|?|
|...|...|...|...|
|78|uint16|Temperature|°C (?)|
|82|uint32|Runtime-total|1h|
|86|?|?|?|
|...|...|...|...|
|110|uint8|0x1B ?|?|
|...|...|...|...|

## Inverter error data (Control code=0x01, FuncCode=0x84)
|Byte offset|Datatype|Description|Unit|
|---|---|---|---|
|0|uint16|VAC low|0.1V|
|2|uint16|VAC high|0.1V|
|4|uint16|FAC low|0.01Hz|
|6|uint16|FAC high|0.01Hz|
|8|?|?|?|
|12|uint16|VAC high slow (?)|0.1V|
|14|uint16|VAC low slow (?)|0.1V|
|16|uint16|?|0.1V|
|18|uint16|?|0.01Hz|
|20|uint16|?|0.01Hz|
|...|...|...|...|
|208|uint16|Settings PIN A|decimal|
|...|...|...|...|

## Values that are missing
* Battery
If you have a battery connected to your inverter -> You can read the values out and print it in a issue.


# ESP8266

## Pinout of ESP
|ESP|USB|
|---|---|
|GND|GND|
|VIN|5V|
|GPI13 (D7) (RXD2)|TX|
|GPI15 (D8)|RX|
  
TODO: add image 

## Design

There are 2 ways how the ESP was programmed. Once as a web server and active wifi connection and MDNS. Of course consumes more power than the second variant with a short setup to the WLAN and then an HTTP post request to an endpoint. After that the WLAN is closed and the ESP goes into deep sleep mode.

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