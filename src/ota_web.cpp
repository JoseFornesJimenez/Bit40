#include "ota_web.h"
#include "../include/gifs/wifi.h"
#include "gif_manager.h" // Necesario para usar la imagen de fondo

const char* default_ssid = "REINACASA";
const char* default_password = "1234";

WebServer server(80);
Preferences preferences;

String scannedNetworks = "";
bool scanInProgress = false;
bool playSelectedGIF = false;
int selectedGIFIndex = -1;

extern GIFManager gifManager; // Usa el gifManager del main
extern GIFEntry gifList[];    // Lista de GIFs disponibles

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
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 10, 10, icon);
  tft.setSwapBytes(false);
}

void startAsyncScan() {
  if (!scanInProgress) {
    WiFi.scanNetworks(true);
    scanInProgress = true;
  }
}

void updateScanResults() {
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED || n == WIFI_SCAN_RUNNING) return;
  scannedNetworks = "";
  for (int i = 0; i < n; ++i) {
    scannedNetworks += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  WiFi.scanDelete();
  scanInProgress = false;
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

    dnsServer.start(53, "*", WiFi.softAPIP());

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
  startAsyncScan();

  server.on("/", HTTP_GET, []() {
    updateScanResults();
    server.send(200, "text/html", R"rawliteral(
      <!DOCTYPE html>
        <html>
        <head>
          <meta name='viewport' content='width=device-width, initial-scale=1.0'>
          <title>BIT40 Config</title>
          <style>
                body {
                font-family: Arial;
                background: #111;
                color: #eee;
                text-align: center;
                padding-top: 50px;
                margin: 0;
              }

              h1 {
                color: #0f0;
                font-size: 32px;
              }

              form {
                margin: 20px auto;
                padding: 20px;
                background: #222;
                width: 90%;
                max-width: 400px;
                border-radius: 10px;
                box-shadow: 0 0 10px #0f0;
              }

              input, select {
                width: 100%;
                padding: 12px;
                margin: 10px 0;
                background: #000;
                color: #0f0;
                border: none;
                font-size: 16px;
                box-sizing: border-box;
              }

              input[type=submit] {
                background: #0f0;
                color: black;
                padding: 12px;
                border-radius: 5px;
                font-size: 16px;
                cursor: pointer;
                width: 100%;
              }

              @media (max-width: 600px) {
                h1 {
                  font-size: 6vw;
                }
                input, select, input[type=submit] {
                  font-size: 4vw;
                }
              }

          </style>
        </head>
        <body>
          <h1>BIT40 Config</h1>
          <form action='/update' method='POST' enctype='multipart/form-data'>
            <input type='file' name='update'><br>
            <input type='submit' value='Actualizar firmware'>
          </form>
          <form action='/wifi' method='POST'>
            <select name='ssid'>
    )rawliteral" + scannedNetworks + R"rawliteral(
                </select><br>
          <input type='password' name='pass' placeholder='Contraseña'><br>
          <input type='submit' value='Guardar WiFi'>
        </form>
        <form action='/resetwifi' method='POST'>
          <input type='submit' value='Borrar configuración WiFi'>
        </form>
        <form id="gifForm">
          <select name='gif' id="gifSelect">
            <option value='0'>Guino</option>
            <option value='1'>Estrella</option>
            <option value='2'>Normal</option>
            <option value='3'>Inicio</option>
            <option value='4'>Mareado</option>
            <option value='5'>Corazones</option>
            <option value='6'>Movimiento1</option>
            <option value='7'>Muerto</option>
            <option value='8'>Baba</option>
          </select>
          <input type='submit' value='Mostrar GIF'>
        </form>
        <div class="info-box">
          <h2>Información del dispositivo</h2>
          <p><strong>Versión firmware:</strong> 1.0.0</p>
          <p><strong>IP actual:</strong> %IP%</p>
          <p><strong>Uptime:</strong> %UPTIME%</p>
        </div>
        <style>
          .info-box {
            margin: 20px auto;
            padding: 20px;
            background: #333;
            color: #0f0;
            width: 90%;
            max-width: 400px;
            border-radius: 10px;
            box-shadow: 0 0 10px #0f0;
            font-size: 16px;
          }

          @media (max-width: 600px) {
            .info-box {
              font-size: 14px;
            }
          }
        </style>

        <p id="gifResponse" style="color:#0f0;"></p>

        <script>
          document.getElementById("gifForm").addEventListener("submit", function(e) {
            e.preventDefault();
            const selected = document.getElementById("gifSelect").value;
            fetch("/setgif", {
              method: "POST",
              headers: { "Content-Type": "application/x-www-form-urlencoded" },
              body: "gif=" + encodeURIComponent(selected)
            })
            .then(response => response.text())
            .then(text => {
              document.getElementById("gifResponse").innerText = text;
            });
          });
        </script>
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

  server.on("/setgif", HTTP_POST, []() {
    if (server.hasArg("gif")) {
      selectedGIFIndex = server.arg("gif").toInt();
      playSelectedGIF = true;
      server.send(200, "text/plain", "GIF seleccionado");
    } else {
      server.send(400, "text/plain", "Falta parámetro gif");
    }
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", Update.hasError() ? "Fallo al actualizar" : "Actualizado OK. Reiniciando...");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      gifManager.setGIF(gifList[MUERTO]);  // Fondo inicial
      gifManager.play();

      Update.onProgress([](size_t progress, size_t total) {
        float percent = (float)progress / total;
        int barWidth = tft.width() - 20;
        int filled = percent * barWidth;
        tft.fillRect(10, 100, barWidth, 10, TFT_DARKGREY);
        tft.fillRect(10, 100, filled, 10, TFT_GREEN);
        tft.setCursor(10, 115);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.printf("%.0f%%", percent * 100);
      });

      Update.begin();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

// Función auxiliar para usarse en loop()
void handleSelectedGIF() {
  if (playSelectedGIF && selectedGIFIndex >= 0) {
    gifManager.setGIF(gifList[selectedGIFIndex]);
    gifManager.play();
    playSelectedGIF = false;
  }
}
