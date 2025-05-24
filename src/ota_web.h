#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

const char* ssid = "REINACASA";
const char* password = "Elpatiodemicasa34";

WebServer server(80);

void setupOTA() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  unsigned long startAttempt = millis();
  const unsigned long timeout = 5000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.println(WiFi.localIP());

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.println("WiFi conectado!");
    tft.setCursor(10, 30);
    tft.println("IP:");
    tft.setCursor(10, 50);
    tft.println(WiFi.localIP());
    delay(3000);
    // Página de carga
    server.on("/", HTTP_GET, []() {
      server.send(200, "text/html", R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
          <title>Actualizar firmware</title>
          <style>
            body {
              font-family: Arial;
              background-color: #111;
              color: #eee;
              display: flex;
              flex-direction: column;
              align-items: center;
              padding-top: 60px;
            }
            h1 {
              color: #00ff99;
            }
            form {
              background: #222;
              padding: 20px;
              border-radius: 8px;
              box-shadow: 0 0 15px #00ff99;
            }
            input[type="file"] {
              margin: 10px 0;
              background: #000;
              color: #00ff99;
              padding: 8px;
              border: none;
            }
            input[type="submit"] {
              background: #00ff99;
              border: none;
              padding: 10px 20px;
              color: black;
              font-weight: bold;
              cursor: pointer;
              border-radius: 4px;
            }
          </style>
        </head>
        <body>
          <h1>BIT40 OTA</h1>
          <form method="POST" action="/update" enctype="multipart/form-data">
            <input type="file" name="update"><br>
            <input type="submit" value="Actualizar firmware">
          </form>
        </body>
        </html>
      )rawliteral");      
    });

    // Manejo del upload
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", Update.hasError() ? "Fallo al actualizar" : "Actualizado OK. Reiniciando...");
      delay(1000);
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Actualizando: %s\n", upload.filename.c_str());
        if (!Update.begin()) {
          Serial.println("Error al iniciar Update");
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Serial.println("Error al escribir");
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.println("Actualización completada");
        } else {
          Serial.printf("Error: %s\n", Update.errorString());
        }
      }
    });

    server.begin();
  } else {
    Serial.println("\nNo se pudo conectar a WiFi.");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.println("No se pudo conectar");
    tft.setCursor(10, 30);
    tft.println("a la red WiFi.");
  }
}
