#ifndef GLOBALS_H
#define GLOBALS_H

#include <WiFi.h>

// shared state
extern int seconds;
extern unsigned long lastSecondTick;
extern bool sendingEnabled;
extern int displayMode;
extern String email;
// sensor values
extern bool floatTriggered;

// web
void handleClient();

#endif
