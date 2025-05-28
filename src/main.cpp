#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TFT_eSPI.h>
#include <AnimatedGIF.h>
#include "gif_manager.h"

// GIFs
#include "gifs/guino.h"
#include "gifs/estrella.h"  // Este será el GIF "mareado"
#include "gifs/inicio.h"
#include "gifs/normal.h"
#include "gifs/mareado.h"
#include "gifs/corazones.h"
#include <gifs/movimiento1.h>
#include <gifs/muerto.h>
#include <gifs/baba.h>
//PRueba
// Primero inicializa el TFT
TFT_eSPI tft = TFT_eSPI();
int num = 0;
// Luego incluye el OTA (usa `tft`)
#include "ota_web.h"

AnimatedGIF gif;
GIFManager gifManager(&tft, &gif);
MPU6050 mpu;
// Lista de GIFs
GIFEntry gifList[] = {
  { guino, sizeof(guino) },         // 0
  { estrella, sizeof(estrella) },   // 1
  { normal, sizeof(normal) },       // 2
  { incio, sizeof(incio) },         // 3 -> inicio
  { mareado, sizeof(mareado) },
  { corazones, sizeof(corazones) },
  { movimiento1 ,sizeof(movimiento1) },
  { muerto ,sizeof(muerto) },
  { baba ,sizeof(baba) }
};


// Índices legibles
enum GifIndex {
  GUINO = 0,
  MAREADO = 4,
  NORMAL = 2,
  INICIO = 3,
  CORAZONES = 5,
  MOVIMIENTO1 = 6,
  ESTRELLA = 1,
  BABA = 8,
  MUERTO = 7
};
int aleatorio[] = {0, 1, 5, 6, 8};

void setup() {
  Serial.begin(115200);

  // ✅ Inicializa TFT antes de usarlo en setupOTA()
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // ✅ Intenta conectar a WiFi y mostrar IP o error
  setupOTA();

  // Inicializa I2C y MPU6050
  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 no conectado");
    while (true);  // Se queda aquí si falla
  }

  // Inicializa GIFs
  gif.begin(BIG_ENDIAN_PIXELS);

  // Muestra animación de inicio
  gifManager.setGIF(gifList[INICIO]);
  gifManager.play();
}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    dnsServer.processNextRequest();
  }
  server.handleClient();           
  handleSelectedGIF();

  static unsigned long lastCheck = 0;
  const unsigned long checkInterval = 300;
  const float shakeThreshold = 1.2;
  const float tiltThreshold = 0.9;

  static int lastGifShown = -1;

  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();

    int16_t ax_raw, ay_raw, az_raw;
    mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);

    float ax = ax_raw / 16384.0;
    float ay = ay_raw / 16384.0;
    float az = az_raw / 16384.0;
    float a = sqrt(ax * ax + ay * ay + az * az);

    Serial.printf("ax: %.2f, ay: %.2f, az: %.2f, a: %.2f\n", ax, ay, az, a);

    int selected = -1;

    if (ay > tiltThreshold) {
      Serial.println("Detectado: BOCA ABAJO → MUERTO");
      selected = MUERTO;
    } else if (abs(ax) > tiltThreshold) {
      Serial.println("Detectado: DE LADO → MAREADO");
      selected = MAREADO;
    } else {
      if (a > shakeThreshold) {
        Serial.println("¡Sacudida detectada!");
        selected = MAREADO;
      } else {
        Serial.println("De pie (normal + aleatorio)");
        gifManager.setGIF(gifList[NORMAL]);
        gifManager.play();

        num = random(0, 6);
        selected = aleatorio[num];
      }
    }

    if (selected != -1 && selected != lastGifShown) {
      gifManager.setGIF(gifList[selected]);
      gifManager.play();
      lastGifShown = selected;
    }
  }

  server.handleClient();
}
