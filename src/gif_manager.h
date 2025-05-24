#pragma once
#include <Arduino.h>
#include <AnimatedGIF.h>
#include <TFT_eSPI.h>

typedef struct {
  const uint8_t* data;
  size_t size;
} GIFEntry;

class GIFManager {
public:
  GIFManager(TFT_eSPI* tft, AnimatedGIF* gif);
  void setGIF(const GIFEntry& gifEntry);
  void play();

private:
  static void draw(GIFDRAW *pDraw);
  static TFT_eSPI* _tft;
  static uint16_t _lineBuffer[256];
  AnimatedGIF* _gif;
  const GIFEntry* _currentGIF;
};
