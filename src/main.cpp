#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <inicio.h>
#include <foto.h> // Debe contener Ojos_abiertos y Ojos_cerrados

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define IMG_WIDTH     128
#define IMG_HEIGHT    44
#define IMG_HEIGHT2    128
#define POS_X         0
#define POS_Y         ((SCREEN_HEIGHT - IMG_HEIGHT) / 2)
#define POS_Y2       0
#define SCLK_PIN GPIO_NUM_8
#define MOSI_PIN GPIO_NUM_10
#define DC_PIN   GPIO_NUM_21
#define CS_PIN   GPIO_NUM_9
#define RST_PIN  GPIO_NUM_20

#define SSD1351_BLACK 0x0000 // Define black color in RGB565 format
#define SSD1351_WHITE 0xFFFF
Adafruit_SSD1351 display = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);

// Buffers en RAM
uint16_t buffer_0[128 * 128];
uint16_t buffer_abiertos[IMG_WIDTH * IMG_HEIGHT];
uint16_t buffer_cerrados[IMG_WIDTH * IMG_HEIGHT];

void copiarDesdePROGMEM(uint16_t* destino, const uint16_t* origen) {
  for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
    uint16_t pixel = pgm_read_word(&origen[i]);

    // Extrae componentes RGB565
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;

    // Calcula brillo aproximado (0-255)
    uint16_t brillo = (r * 255 / 31 + g * 255 / 63 + b * 255 / 31) / 3;

    // Si el píxel es suficientemente claro, hazlo blanco
    if (brillo > 180) {
      destino[i] = 0xFFFF; // Blanco puro
    } else {
      destino[i] = 0x0000; // Negro (puedes poner un umbral más fino si quieres conservar grises)
    }
  }
}
void copiarDesdePROGMEM1(uint16_t* destino, const uint16_t* origen) {
  for (int i = 0; i < 128 * 128; i++) {
    uint16_t pixel = pgm_read_word(&origen[i]);

    // Extrae componentes RGB565
    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5) & 0x3F;
    uint8_t b = pixel & 0x1F;

    // Calcula brillo aproximado (0-255)
    uint16_t brillo = (r * 255 / 31 + g * 255 / 63 + b * 255 / 31) / 3;

    // Si el píxel es suficientemente claro, hazlo blanco
    if (brillo > 180) {
      destino[i] = 0x0000; // Blanco puro
    } else {
      
      destino[i] = 0xFFFF; // Negro (puedes poner un umbral más fino si quieres conservar grises)
    }
  }
}



void setup() {
  display.begin();
  display.fillScreen(SSD1351_BLACK);

  // Copiar imágenes desde PROGMEM a RAM (solo una vez)

  copiarDesdePROGMEM1(buffer_0, epd_bitmap_0);
  copiarDesdePROGMEM(buffer_abiertos, Ojos_abiertos);
  copiarDesdePROGMEM(buffer_cerrados, Ojos_cerrados);
  display.fillScreen(SSD1351_BLACK);


  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_1);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_0);
  delay(20);
  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_1);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_0);
  delay(20);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_3);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_5);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_7);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_9);
  delay(10);

  display.drawRGBBitmap(0, 0, buffer_0, 128, 128);
  delay(10);
}

void loop() {
  
  for (int i = 0; i < 7; i++) {
    
    display.drawRGBBitmap(POS_X, POS_Y, buffer_abiertos, IMG_WIDTH, IMG_HEIGHT );
    delay(10);
  }

  for (int i = 0; i < 1; i++) {
    display.drawRGBBitmap(POS_X, POS_Y, buffer_cerrados, IMG_WIDTH, IMG_HEIGHT);
    delay(10);
  }
}
