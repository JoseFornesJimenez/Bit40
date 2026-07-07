#include "comm_manager.h"
#include <esp_now.h>
#include <WiFi.h>
#include "ota_web.h"
#include <vector>

// Definir foundPeers como variable global en este archivo
std::vector<EspNowPeer> foundPeers;

void asyncScanEspNowPeers() {
    foundPeers.clear();
    int16_t numNetworks = WiFi.scanNetworks(false, true); // Solo los argumentos válidos
    for (int i = 0; i < numNetworks; ++i) {
        if (WiFi.BSSID(i)) {
            EspNowPeer peer;
            memcpy(peer.mac, WiFi.BSSID(i), 6);
            char buf[18];
            sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", peer.mac[0], peer.mac[1], peer.mac[2], peer.mac[3], peer.mac[4], peer.mac[5]);
            peer.macStr = String(buf);
            foundPeers.push_back(peer);
        }
    }
}

String getPeersJson() {
    String json = "[";
    for (size_t i = 0; i < foundPeers.size(); ++i) {
        json += "{\"mac\":\"" + foundPeers[i].macStr + "\"}";
        if (i < foundPeers.size() - 1) json += ",";
    }
    json += "]";
    return json;
}

void CommManager::begin() {
    initEspNow();
    initWiFi();
}

void CommManager::loop() {
    if (comm_mode == USE_ESPNOW) {
        if (!espnowPing()) {
            comm_mode = USE_WIFI;
            connectToWiFi();
            connectToServer();
        } else {
            sendEspNowMessage("Hola!");
        }
    } else if (comm_mode == USE_WIFI) {
        if (espnowPing()) {
            comm_mode = USE_ESPNOW;
            disconnectFromWiFi();
        } else {
            sendServerMessage("Hola por WiFi!");
        }
    }
}

int CommManager::getMode() {
    return comm_mode;
}

void CommManager::sendMessage(const String& msg) {
    if (comm_mode == USE_ESPNOW) {
        sendEspNowMessage(msg);
    } else {
        sendServerMessage(msg);
    }
}

// --- Métodos vacíos para implementar ---
bool CommManager::espnowPing() { return false; }
void CommManager::initEspNow() {}
void CommManager::sendEspNowMessage(const String&) {}
void CommManager::initWiFi() {}
void CommManager::connectToWiFi() {}
void CommManager::connectToServer() {}
void CommManager::sendServerMessage(const String&) {}
void CommManager::disconnectFromWiFi() {}
