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

TFT_eSPI tft = TFT_eSPI();
AnimatedGIF gif;
GIFManager gifManager(&tft, &gif);
MPU6050 mpu;

GIFEntry gifList[] = {
  { guino, sizeof(guino) },         // 0
  { estrella, sizeof(estrella) },   // 1
  { normal, sizeof(normal) },       // 2
  { incio, sizeof(incio) },          // 3 -> inicio
  { mareado, sizeof(mareado) }          // 4 -> mareado
};

enum GifIndex {
  GUINO = 0,
  MAREADO = 4,
  NORMAL = 2,
  INICIO = 3
};

void setup() {
  Serial.begin(115200);
  Wire.begin();          // SDA=21, SCL=22 por defecto
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 no conectado");
    while (true);
  }

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  gif.begin(BIG_ENDIAN_PIXELS);

  // Mostrar animación de inicio
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
    } else {
      gifManager.setGIF(gifList[NORMAL]);
      gifManager.play();
    }
  }
}
