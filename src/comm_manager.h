#pragma once
#include <Arduino.h>
#include <vector>

#define USE_ESPNOW 1
#define USE_WIFI   2

struct EspNowPeer {
    uint8_t mac[6];
    String macStr;
};

void asyncScanEspNowPeers();
String getPeersJson();

class CommManager {
public:
    void begin();
    void loop();
    int getMode();
    void sendMessage(const String& msg);
private:
    int comm_mode = USE_ESPNOW;
    bool espnowPing();
    void initEspNow();
    void sendEspNowMessage(const String& msg);
    void initWiFi();
    void connectToWiFi();
    void connectToServer();
    void sendServerMessage(const String& msg);
    void disconnectFromWiFi();
};
