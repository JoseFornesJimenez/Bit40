#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <inicio.h>
#include <foto.h> // Debe contener Ojos_abiertos y Ojos_cerrados

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128
#define IMG_WIDTH     128
#define IMG_HEIGHT    44
#define POS_X         0
#define POS_Y         ((SCREEN_HEIGHT - IMG_HEIGHT) / 2)
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

void copiarDesdePROGMEM1(uint16_t* destino, const uint16_t* origen, int weight = 128, int height = 128) {
  for (int i = 0; i < weight * height; i++) {
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


void mostrarFrames(const uint16_t* frames[], int numFrames, int delayMs=10, int posx = 0, int posy = 0, int weigh = 128, int height = 128, int pasador = 1) {
  for (int i = 0; i < numFrames; i++) {
    if(pasador == 0 ){copiarDesdePROGMEM(buffer_0, frames[i]);}
    else{copiarDesdePROGMEM1(buffer_0, frames[i]);}
    display.drawRGBBitmap(posx, posy, buffer_0, weigh, height);
    delay(delayMs);
  }
}



void setup() {
  
  randomSeed(analogRead(0)); // Usa una lectura analógica como semilla
  display.begin();
  display.fillScreen(SSD1351_BLACK);
  copiarDesdePROGMEM1(buffer_0, epd_bitmap_0);
  copiarDesdePROGMEM(buffer_abiertos, Ojos_abiertos);
  copiarDesdePROGMEM(buffer_cerrados, Ojos_cerrados);
  //las secuencias estan en inicio.h
  mostrarFrames(secuencia, sizeof(secuencia) / sizeof(secuencia[0]), 10);
}

void loop() {
  int animacion = random(0, 6);
  const uint16_t* ojos1[] = {
    Ojos_abiertos,
  };
  // Mostrar ojos en la posición original
  mostrarFrames(ojos, sizeof(ojos) / sizeof(ojos[0]), 5, POS_X, POS_Y, IMG_WIDTH, IMG_HEIGHT, 0);
  if(animacion == 2){
    for(int i = 0; i < 10; i+=2){
      mostrarFrames(ojos1, sizeof(ojos1) / sizeof(ojos1[0]), 5, POS_X+i, POS_Y+i, IMG_WIDTH, IMG_HEIGHT, 0);
    }
    for(int i = 10; i > 0; i-=2){
      mostrarFrames(ojos1, sizeof(ojos1) / sizeof(ojos1[0]), 5, POS_X+i, POS_Y+i, IMG_WIDTH, IMG_HEIGHT, 0);
    }
    for(int i = 0; i < 10; i+=2){
      mostrarFrames(ojos1, sizeof(ojos1) / sizeof(ojos1[0]), 5, POS_X-i, POS_Y-i, IMG_WIDTH, IMG_HEIGHT, 0);
    }
    for(int i = 10; i > 0; i-=2){
      mostrarFrames(ojos1, sizeof(ojos1) / sizeof(ojos1[0]), 5, POS_X-i, POS_Y-i, IMG_WIDTH, IMG_HEIGHT, 0);
    }
  }

  // // Limpiar esa zona
  // display.fillRect(POS_X, POS_Y, IMG_WIDTH, IMG_HEIGHT, SSD1351_BLACK);

  // // Mostrar ojos en la posición desplazada
  // mostrarFrames(ojos, sizeof(ojos) / sizeof(ojos[0]), 5, POS_X, POS_Y + 10, IMG_WIDTH, IMG_HEIGHT, 0);
  // // Limpiar esa zona también
  // display.fillRect(POS_X, POS_Y + 10, IMG_WIDTH, IMG_HEIGHT, SSD1351_BLACK);
}

