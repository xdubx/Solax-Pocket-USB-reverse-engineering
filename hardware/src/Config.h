#ifndef __CONFIG_H__
#define __CONFIG_H__



// Should be the Webserver enabled
//#define workModeAPI

// Should be HTTP Request enabled
#define WEB_REQUEST_SUPPORTED 1

// Should be MQTT enabled
#define MQTT_SUPPORTED 1

/**
 * only one of both modes are posible if you activate both the controller crashed Serial1 and buildin led are on pin 2
 * 
 */
//#define debugSERIAL
//#define debugLED

// Data of the Wifi access point
// Default IP 192.168.4.1
#define HOSTNAME          "Solax"
#define APPassword        "solaxsolar"





#endif 