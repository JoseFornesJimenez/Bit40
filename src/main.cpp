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

// Primero inicializa el TFT
TFT_eSPI tft = TFT_eSPI();

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
  { mareado, sizeof(mareado) }      // 4 -> mareado
};

// Índices legibles
enum GifIndex {
  GUINO = 0,
  MAREADO = 4,
  NORMAL = 2,
  INICIO = 3
};

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
  static unsigned long lastCheck = 0;
  const unsigned long checkInterval = 300;  // ms
  const float threshold = 1.2;              // Umbral de sacudida en g

  if (millis() - lastCheck > checkInterval) {
    lastCheck = millis();

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float a = sqrt(ax * ax + ay * ay + az * az) / 16384.0;

    if (a > threshold) {
      Serial.println("¡Sacudida detectada!");
      gifManager.setGIF(gifList[MAREADO]);
      gifManager.play();
      gifManager.setGIF(gifList[MAREADO]);
      gifManager.play();
      gifManager.setGIF(gifList[MAREADO]);
      gifManager.play();
    } else {
      int num = random(0, 4);
      gifManager.setGIF(gifList[NORMAL]);
      gifManager.play();
      if(num >= 0 && num <= 1){
        gifManager.setGIF(gifList[num]);
        gifManager.play();
      }
    }
  }

  // ✅ Necesario para que funcione la web OTA
  server.handleClient();
}
