# Holographic Volumetric LED Display

A DIY ESP32-based volumetric LED display that receives a bitmap via HTTP, processes it into radial slices, and maps the pixels to a set of LEDs for POV 3D visualization.

---

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Hardware Requirements](#hardware-requirements)
* [Software Requirements](#software-requirements)
* [JSON Image Format](#json-image-format)
* [Setup Instructions](#setup-instructions)
* [Usage](#usage)
* [Code Structure](#code-structure)
* [Future Improvements](#future-improvements)

---

## Overview

This project implements a simple HTTP server on an ESP32 in AP mode. It accepts a binary bitmap (2D array of 0/1) via a POST request, processes the image into radial slices over a configurable number of angles, and then compresses each slice into a lower-resolution LED frame. The resulting `ledFrame` array can be displayed on a rotating strip of LEDs for a persistence-of-vision volumetric effect.

---

## Features

* ESP32 runs as a Wi-Fi Access Point and HTTP server
* Receives bitmap images in JSON format
* Dynamic handling of arbitrary image dimensions (up to configured limits)
* Radial slicing into a configurable number of angles (`NUM_ANGLES`)
* Down-sampling from `numPixels` samples to `NUM_LEDS` on/off LED states per slice
* JSON-based feedback for status and error handling

---

## Hardware Requirements

* ESP32 development board
* LED strip (e.g., WS2812) with at least `NUM_LEDS` pixels
* Motor and mechanical assembly for rotating the LED strip (holographic fan)
* Magnet + Hall sensor for rotation synchronization (optional)

---

## Software Requirements

* Arduino IDE or PlatformIO
* ArduinoJson library
* WiFi and WebServer libraries (included with ESP32 core)

---

## JSON Image Format

The HTTP endpoint `/uploadImage` expects a JSON payload with the following structure:

```json
{
  "width": <int>,        // image width in pixels
  "height": <int>,       // image height in pixels
  "pixels": [            // 2D array of 0/1 values
    [0,1,0,...],
    [1,0,1,...],
    ...
  ]
}
```

* `width` and `height` must match the dimensions of the `pixels` array.
* Each row in `pixels` must have exactly `width` entries.
* The total number of rows must equal `height`.

---

## Setup Instructions

1. **Clone the repository**

   ```bash
   git clone <repo-url>
   cd volumetric-led-display
   ```

2. **Install dependencies**

   * Open Arduino IDE
   * Go to `Tools > Manage Libraries...`
   * Install `ArduinoJson`

3. **Configure constants**

   * In `main.ino`, adjust `NUM_LEDS`, `MAX_WIDTH`, `MAX_HEIGHT`, and `NUM_ANGLES` as needed.

4. **Upload firmware**

   * Select your ESP32 board and COM port
   * Click **Upload**

5. **Connect to the Access Point**

   * SSID: `HolographicEngine`
   * No password required

6. **Send image data**

   * Use `curl` or a custom script to POST JSON to `http://192.168.4.1/uploadImage`

---

## Usage

Example using `curl`:

```bash
curl -X POST \
  http://192.168.4.1/uploadImage \
  -H "Content-Type: application/json" \
  -d @bitmap.json
```

* The ESP32 will parse the JSON, process the bitmap, and print each LED frame slice to the Serial console.
* Response:

  ```json
  {"status":"bitmap processed"}
  ```

---

## Code Structure

* **setupAccessPoint()**: Initializes ESP32 as a Wi-Fi AP.
* **setupHttpServer()**: Defines HTTP routes and starts the server.
* **handleUploadImage()**: Parses JSON, fills `bitmap[][]`, computes radial slices, and calls `convertPixelImageToLedMap()`.
* **convertPixelImageToLedMap()**: Down-samples a radial pixel array into `NUM_LEDS` LED states.

---

## Future Improvements

* Integrate `FastLED` to output `ledFrame[]` directly to the LED strip
* Add Hall sensor interrupt handling for precise angle synchronization
* Support animated sequences by queuing multiple frames
* Optimize memory usage for larger images or higher resolution
