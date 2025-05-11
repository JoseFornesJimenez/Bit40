# Bit40 - Mascota Virtual Retro

Bit40 es un proyecto de mascota virtual retro diseñado para pantallas OLED de 0.96 pulgadas. Inspirado en las antiguas interfaces monocromáticas, Bit40 muestra expresiones faciales simples y reactivas utilizando gráficos en blanco y negro. Ideal para pequeños dispositivos embebidos como ESP32-C3.

## 🚀 Funcionalidades

* Cara retro compuesta por ojos, boca y nariz.
* Animaciones y expresiones simples.
* Estilo monocromático de 8 bits.
* Interfaz personalizable para futuras expansiones.

## 📦 Estructura del Proyecto

```
Bit40/
│── src/
│   └── main.cpp   # Código principal
│── lib/           # Librerías adicionales
│── platformio.ini # Configuración de PlatformIO
│── .gitignore     # Archivos ignorados por Git
│── README.md      # Este archivo
```

## 🛠️ Requisitos

* Visual Studio Code con PlatformIO
* Placa ESP32-C3
* Pantalla OLED de 0.96" (I2C)

### Librerías necesarias:

* Adafruit SSD1306
* Adafruit GFX Library

## ⚡ Instalación

1. Clonar el repositorio:

   ```bash
   git clone https://github.com/tu-usuario/Bit40.git
   cd Bit40
   ```

2. Instalar las librerías:

   * Abre PlatformIO y selecciona "Library Manager".
   * Busca e instala las librerías `Adafruit SSD1306` y `Adafruit GFX Library`.

3. Compilar y subir el código:

   * Conecta la placa ESP32-C3.
   * Haz clic en el icono de check (Build).
   * Haz clic en el icono de flecha (Upload).

## ✅ Uso

* Una vez cargado el código, la pantalla mostrará la cara retro de Bit40.
* Puedes modificar las expresiones y agregar nuevas animaciones en el archivo `main.cpp`.

## 📝 Notas

Este proyecto está en fase inicial. Las futuras actualizaciones incluirán:

* Más animaciones.
* Interacciones por botones o sensores.
* Control por Bluetooth o WiFi.

---

¡Contribuciones, ideas y sugerencias son bienvenidas! 🚀
