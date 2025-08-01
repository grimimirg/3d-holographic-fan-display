#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <math.h>

// ----------------------------------------------------------------
// CONSTANTS
const int NUM_LEDS    = 10; // number of LEDs along the arm

const int MAX_WIDTH   = 200;
const int MAX_HEIGHT  = 200;

const int NUM_ANGLES  = 360;

// HTTP responses
enum class HttpStatus {
  OK          = 200,
  BAD_REQUEST = 400,
  NOT_FOUND   = 404
};

const char *contentType = "application/json";

// ----------------------------------------------------------------
// GLOBALS
WebServer server(80);

static uint8_t bitmap[MAX_HEIGHT][MAX_WIDTH];
static uint8_t pixelFrame[MAX_WIDTH/2 > MAX_HEIGHT/2 ? MAX_WIDTH/2 : MAX_HEIGHT/2];
static uint8_t ledFrame[NUM_LEDS];

/* --------------------------------
   SETUP
   -------------------------------- */
void setup() {
  Serial.begin(115200);
  setupAccessPoint();
  setupHttpServer();
}

/* --------------------------------
   LOOP
   -------------------------------- */
void loop() {
  server.handleClient();
}

/* --------------------------------
   FUNCTIONS
   -------------------------------- */
void setupAccessPoint() {
  Serial.println("Starting Access Point...");
  if (!WiFi.softAP("Holographic Engine (AccessPoint)", "")) {
    Serial.println("AP creation failed!");
    while (true) delay(1000);
  }
  Serial.print("AP ready, IP: ");
  Serial.println(WiFi.softAPIP());
}

void setupHttpServer() {
  server.on("/uploadImage", HTTP_POST, handleUploadImage);
  server.onNotFound([]() {
    server.send((int)HttpStatus::NOT_FOUND, contentType, "{\"error\":\"Route not found\"}");
  });
  server.begin();
}

void handleUploadImage() {
  if (!server.hasArg("plain")) {
    server.send((int)HttpStatus::BAD_REQUEST, contentType, "{\"error\":\"Missing JSON body\"}");
    return;
  }

  StaticJsonDocument<20*1024> doc;
  auto err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send((int)HttpStatus::BAD_REQUEST, contentType, "{\"error\":\"JSON parse error\"}");
    return;
  }

  int width  = doc["width"];
  int height = doc["height"];
  if (width <= 0 || width > MAX_WIDTH ||
      height <= 0 || height > MAX_HEIGHT) {
    server.send((int)HttpStatus::BAD_REQUEST, contentType, "{\"error\":\"Invalid dimensions\"}");
    return;
  }

  JsonArray rows = doc["pixels"];
  if ((int)rows.size() != height) {
    server.send((int)HttpStatus::BAD_REQUEST, contentType, "{\"error\":\"Wrong number of rows\"}");
    return;
  }

  for (int y = 0; y < height; y++) {
    JsonArray cols = rows[y].as<JsonArray>();

    if ((int)cols.size() != width) {
      server.send((int)HttpStatus::BAD_REQUEST, contentType, "{\"error\":\"Wrong number of columns in row\"}");
      return;
    }

    for (int x = 0; x < width; x++) {
      bitmap[y][x] = cols[x] ? 1 : 0;
    }
  }

  float cx = width  / 2.0f;
  float cy = height / 2.0f;

  int numPixels = min(width, height) / 2;  // until the edge of the image

  if (numPixels < 1) numPixels = 1;

  for (int a = 0; a < NUM_ANGLES; a++) {
    float theta = a * (2.0f * PI / NUM_ANGLES);

    for (int r = 0; r < numPixels; r++) {
      int xi = int(cx + r * cos(theta) + 0.5f);
      int yi = int(cy + r * sin(theta) + 0.5f);

      xi = constrain(xi, 0, width  - 1);
      yi = constrain(yi, 0, height - 1);
      pixelFrame[r] = bitmap[yi][xi];
    }

    convertPixelImageToLedMap(pixelFrame, ledFrame, numPixels, NUM_LEDS);

    // TODO: use somehow ledFrame into FastLED.show()

  }

  server.send((int)HttpStatus::OK, contentType, "{\"status\":\"bitmap processed\"}");
}

void convertPixelImageToLedMap(const uint8_t *pixels, uint8_t *leds, int numPixels, int numLeds) {
  int blockSize = numPixels / numLeds;

  for (int block = 0; block < numLeds; block++) {
    bool anyOn = false;
    int start = block * blockSize;
    int end   = start + blockSize;

    if (block == numLeds - 1) {
      end = numPixels;
    }

    for (int i = start; i < end; i++) {
      if (pixels[i]) {
        anyOn = true;
        break;
      }
    }

    leds[block] = anyOn ? 1 : 0;
  }
}
