#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <Preferences.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern WebServer server;
extern Preferences preferences;

extern const char* default_ssid;
extern const char* default_password;

void saveWiFiConfig(const String& ssid, const String& password);
bool loadWiFiConfig(String& ssid, String& password);
void clearWiFiConfig();
void drawWiFiStatusIcon();
void setupOTA();
