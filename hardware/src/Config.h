#ifndef __CONFIG_H__
#define __CONFIG_H__



// Should be the Webserver enabled
//#define workModeAPI

// Should be HTTP Request enabled
#define WEB_REQUEST_SUPPORTED 1

// Should be MQTT enabled
#define MQTT_SUPPORTED 1

// Should be the webserver enabled
#define WEBSERVER_SUPPORTED 1

/**
 * only one of both modes are posible if you activate both the controller crashed Serial1 and buildin led are on pin 2
 * 
 */
//#define debugSERIAL
//#define ENABLE_DEBUG_OUTPUT
#define ENABLE_WEB_DEBUG 1

// Data of the Wifi access point
// Default IP 192.168.4.1
#define HOSTNAME          "Solax"
#define APPassword        "solaxsolar"

// How often should the data be pulled
#define REFRESH_TIMER 5000 // 5s default


#endif 