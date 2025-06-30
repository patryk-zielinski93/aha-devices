#ifndef AHA_DEVICES_DEBUG_H
#define AHA_DEVICES_DEBUG_H

// #define DEBUG
// #define DEBUGSTACK

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

#endif // AHA_DEVICES_DEBUG_H