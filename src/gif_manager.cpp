#include "gif_manager.h"

TFT_eSPI* GIFManager::_tft = nullptr;
uint16_t GIFManager::_lineBuffer[256];

GIFManager::GIFManager(TFT_eSPI* tft, AnimatedGIF* gif) {
  _tft = tft;
  _gif = gif;
  _currentGIF = nullptr;
}

void GIFManager::setGIF(const GIFEntry& gifEntry) {
  _currentGIF = &gifEntry;
}

void GIFManager::play() {
  if (!_currentGIF) return;

  if (_gif->open((uint8_t*)_currentGIF->data, _currentGIF->size, draw)) {
    _tft->startWrite();
    while (_gif->playFrame(true, NULL)) {
      yield();
    }
    _gif->close();
    _tft->endWrite();
  }
}

void GIFManager::draw(GIFDRAW *pDraw) {
  if (pDraw->y >= _tft->height()) return;

  int width = pDraw->iWidth;
  if (width + pDraw->iX > _tft->width()) width = _tft->width() - pDraw->iX;

  if (!pDraw->ucHasTransparency) {
    uint8_t *s = pDraw->pPixels;
    for (int i = 0; i < width; i++) {
      _lineBuffer[i] = pDraw->pPalette[*s++];
    }
    _tft->setAddrWindow(pDraw->iX, pDraw->iY + pDraw->y, width, 1);
    _tft->pushPixels(_lineBuffer, width);
  }
}
