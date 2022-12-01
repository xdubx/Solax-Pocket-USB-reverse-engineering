#ifndef __CONFIG_H__
#define __CONFIG_H__

/**
 * @brief set the workmode of the esp
 *  api endpoint over http server
 *  send the values as json over http post
 */
//#define workModeAPI
#define workModeSEND

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