#include "ota_web.h"
#include "../include/gifs/wifi.h"
const char* default_ssid = "REINACASA";
const char* default_password = "Elpatiodemicasa34";

WebServer server(80);
Preferences preferences;

void saveWiFiConfig(const String& ssid, const String& password) {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", password);
  preferences.end();
}

bool loadWiFiConfig(String& ssid, String& password) {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  preferences.end();
  return ssid.length() > 0;
}

void clearWiFiConfig() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
}

void drawWiFiStatusIcon() {
    const uint16_t* icon = WiFi.status() == WL_CONNECTED ? iconwifi_ok : iconwifi_off;
    tft.setSwapBytes(true);  // Asegúrate de que el formato sea correcto si es RGB565
    tft.pushImage(0, 0, 10, 10, icon);
    tft.setSwapBytes(false);
  }

void setupOTA() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  String ssid, pass;
  if (!loadWiFiConfig(ssid, pass)) {
    ssid = default_ssid;
    pass = default_password;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  const unsigned long timeout = 5000;

  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(250);
  }

  // Mostrar pantalla de estado de conexión antes de continuar
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 10);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.println(WiFi.localIP());
    tft.setCursor(10, 10);
    tft.println("WiFi conectado!");
    tft.setCursor(10, 30);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    delay(2000);
  } else {
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("BIT40_CONFIG");

    IPAddress IP = WiFi.softAPIP();
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("Modo config AP");
    tft.setCursor(10, 30);
    tft.println("Red: BIT40_CONFIG");
    tft.setCursor(10, 50);
    tft.print("IP: ");
    tft.println(IP);
  }

  delay(5000);

  // Web UI
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head><title>BIT40 Config</title>
          <style>
            body { font-family: Arial; background: #111; color: #eee; text-align: center; padding-top: 50px; }
            h1 { color: #0f0; }
            form { margin: 20px auto; padding: 20px; background: #222; width: 300px; border-radius: 10px; box-shadow: 0 0 10px #0f0; }
            input[type=text], input[type=password], input[type=file] {
              width: 90%; padding: 10px; margin: 10px 0; background: #000; color: #0f0; border: none;
            }
            input[type=submit] {
              background: #0f0; color: black; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer;
            }
          </style>
        </head>
        <body>
          <h1>BIT40 Config</h1>
          <form action="/update" method="POST" enctype="multipart/form-data">
            <input type="file" name="update"><br>
            <input type="submit" value="Actualizar firmware">
          </form>
          <form action="/wifi" method="POST">
            <input type="text" name="ssid" placeholder="Nuevo SSID"><br>
            <input type="password" name="pass" placeholder="Nueva contraseña"><br>
            <input type="submit" value="Guardar WiFi">
          </form>
          <form action="/resetwifi" method="POST">
            <input type="submit" value="Borrar configuración WiFi">
          </form>
        </body>
        </html>
      )rawliteral");
    });

  server.on("/wifi", HTTP_POST, []() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
      saveWiFiConfig(server.arg("ssid"), server.arg("pass"));
      server.send(200, "text/plain", "Guardado. Reiniciando...");
      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Faltan campos");
    }
  });

  server.on("/resetwifi", HTTP_POST, []() {
    clearWiFiConfig();
    server.send(200, "text/plain", "WiFi borrado. Reinicia.");
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", Update.hasError() ? "Fallo al actualizar" : "Actualizado OK. Reiniciando...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Update.begin();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });

  server.begin();
}
