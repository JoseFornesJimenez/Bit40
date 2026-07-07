#include "gif_manager.h"
#include "ota_web.h" // Necesario para acceso a drawWiFiStatusIcon

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
      drawWiFiStatusIcon(); // <-- Siempre dibuja el icono encima de cada frame
      yield();
    }
    _gif->close();
    _tft->endWrite();
  }
}

void GIFManager::draw(GIFDRAW *pDraw) {
  const int FRANJA_SUPERIOR = 16; // Altura de la franja superior (ajusta si es necesario)
  if (pDraw->y >= _tft->height()) return;

  // Evita dibujar en la franja superior
  if ((pDraw->y + pDraw->iY) < FRANJA_SUPERIOR) return;

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

  // Si la línea afecta a la franja, repinta iconos (opcional, según tu lógica)
  if (pDraw->y + pDraw->iY <= FRANJA_SUPERIOR) {
    drawWiFiStatusIcon();
  }
}

