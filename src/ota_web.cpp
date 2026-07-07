#include "ota_web.h"
#include "../include/gifs/wifi.h"
#include "gif_manager.h" // Necesario para usar la imagen de fondo
#include "comm_manager.h"
#include <time.h>

const char* default_ssid = "REINACASA";
const char* default_password = "1234";

WebServer server(80);
Preferences preferences;
DNSServer dnsServer;
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

  // --- Dibuja la batería en la esquina superior derecha ---
  static float smoothPercent = 0;
  static int lastPercent = -1;
  static uint16_t lastColor = 0xFFFF;
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(A0);
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0;
  int percent = map(Vbattf * 1000, 3300, 4200, 0, 100);
  percent = constrain(percent, 0, 100);

  // Detecta si está enchufado el USB (ajusta el método según tu hardware)
  bool usbPlugged = false;
  #ifdef ARDUINO_USB_CDC_ON_BOOT
    usbPlugged = Serial; // En algunos ESP32 esto indica USB conectado
  #else
    // Alternativa: si el voltaje es muy alto, probablemente está cargando
    if (Vbattf > 4.15) usbPlugged = true;
  #endif

  // Suavizado simple (promedio móvil exponencial)
  if (smoothPercent == 0) smoothPercent = percent;
  smoothPercent = 0.8 * smoothPercent + 0.2 * percent;
  int percentToShow = round(smoothPercent);

  int battW = 24, battH = 10;
  int16_t screenW = 128; // Ajusta si tu pantalla es de otro ancho
  int battX = screenW - battW - 25;
  int battY = 0;
  uint16_t color;
  if (usbPlugged) {
    color = TFT_YELLOW;
  } else if (percentToShow < 20) {
    color = TFT_RED;
  } else {
    color = TFT_GREEN;
  }
  // Repinta si cambia el color o el porcentaje
  if (lastPercent != percentToShow || lastColor != color) {
    tft.fillRect(battX-2, battY, battW+28, battH, TFT_BLACK);
    lastPercent = percentToShow;
    lastColor = color;
  }
  tft.drawRect(battX, battY, battW, battH, TFT_WHITE); // Marco
  tft.fillRect(battX + battW, battY + battH/4, 3, battH/2, TFT_WHITE); // Terminal
  int fillW = map(percentToShow, 0, 100, 0, battW-2);
  tft.fillRect(battX+1, battY+1, fillW, battH-2, color);
  // Dibuja el porcentaje a la derecha del icono
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(battX + battW + 6, battY);
  tft.printf("%d%%", percentToShow);

  // --- Dibuja la hora en la esquina inferior derecha ---
  static char lastTimeStr[6] = "";
  char timeStr[6] = "--:--";
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  }
  int16_t hourX = screenW - 34;
  int16_t hourY = 240 - 10; // Ajusta si tu pantalla es de otro altura
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  if (strcmp(lastTimeStr, timeStr) != 0) {
    tft.fillRect(hourX, hourY, 34, 10, TFT_BLACK); // Limpia fondo solo si cambia
    strcpy(lastTimeStr, timeStr);
  }
  tft.setCursor(hourX, hourY);
  tft.print(timeStr);
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
    tft.fillRect(0, 0, tft.width(), 16, TFT_BLACK);

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

  configTime(7200, 0, "pool.ntp.org", "time.nist.gov"); // UTC+2 (7200 segundos)

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
        <form id="espnowForm">
          <button type="button" id="scanPeersBtn">Escanear ESP-NOW cercanos</button>
          <select id="peerSelect" name="peer">
            <option value="">Selecciona un dispositivo</option>
          </select>
          <input type="submit" value="Conectar con este ESP-NOW">
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
        <p id="espnowResponse" style="color:#0f0;"></p>

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
          document.getElementById("scanPeersBtn").onclick = function() {
            fetch("/scan_peers", {method: "POST"})
              .then(() => setTimeout(loadPeers, 1000));
          };
          function loadPeers() {
            fetch("/peers")
              .then(r => r.json())
              .then(peers => {
                const sel = document.getElementById("peerSelect");
                sel.innerHTML = '<option value="">Selecciona un dispositivo</option>';
                peers.forEach(peer => {
                  sel.innerHTML += `<option value="${peer.mac}">${peer.mac}</option>`;
                });
              });
          }
          document.getElementById("espnowForm").onsubmit = function(e) {
            e.preventDefault();
            const mac = document.getElementById("peerSelect").value;
            if (!mac) return;
            fetch("/set_peer", {
              method: "POST",
              headers: {"Content-Type": "application/x-www-form-urlencoded"},
              body: "mac=" + encodeURIComponent(mac)
            })
            .then(r => r.text())
            .then(txt => document.getElementById("espnowResponse").innerText = txt);
          };
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
      gifManager.setGIF(gifList[7]);  // Fondo inicial
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

  // Endpoint para iniciar escaneo asíncrono de ESP-NOW
  server.on("/scan_peers", HTTP_POST, []() {
    asyncScanEspNowPeers();
    server.send(200, "application/json", "{\"status\":\"scanning\"}");
  });

  // Endpoint para obtener el listado de peers encontrados
  server.on("/peers", HTTP_GET, []() {
    server.send(200, "application/json", getPeersJson());
  });

  // Endpoint para guardar la MAC seleccionada
  server.on("/set_peer", HTTP_POST, []() {
    if (server.hasArg("mac")) {
      preferences.begin("espnow", false);
      preferences.putString("peer_mac", server.arg("mac"));
      preferences.end();
      server.send(200, "text/plain", "MAC guardada: " + server.arg("mac"));
    } else {
      server.send(400, "text/plain", "Falta la MAC");
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
