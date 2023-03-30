#include <Wire.h>
#include <Adafruit_VL53L1X.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// WiFi settings
const char* ssid = "Athenaeum";
const char* password = "community";

// UDP settings
WiFiUDP udp;
unsigned int localPort = 4210;
const char* remoteIP = "10.0.0.30";
unsigned int remotePort = 8850;
char packetBuffer[255];
String message;

// Node ID
const int nodeID = 1;

// Digital IO button
const int buttonPin = 15;
bool buttonState = 0;

// Proximity sensor
Adafruit_VL53L1X vls3dx_sensor1 = Adafruit_VL53L1X();
// Adafruit_VL53L1X vls3dx_sensor2 = Adafruit_VL53L1X();
const int proximitySensorAddress = 0x29;

// LED strip settings
const int ledPin = 13;
const int stripLength = 30;
Adafruit_NeoPixel strip(stripLength, ledPin, NEO_GRB + NEO_KHZ800);

// Triple buffering for LED color data
uint32_t ledColorBuffer1[stripLength];
uint32_t ledColorBuffer2[stripLength];
uint32_t ledColorBuffer3[stripLength];
int bufferSelect = 0;

// Frame rate for LED display
const int frameRate = 60;

// Time variables for interpolation and fallback mode
unsigned long lastFrameTime = 0;
unsigned long lastReceivedPacketTime = 0;

// Fallback mode timeout (in milliseconds)
const unsigned long fallbackTimeout = 20000;

// Function declarations
// setup:
void connectToWiFi();
// receive:
void receiveWakeup();
void receiveSleep();
void receiveLEDData();
void receiveConfigData();
// tasks:
void coreTask0(void* pvParameters);
void coreTask1(void* pvParameters);
// LEDs:
void updateLEDColorBuffer(int index, uint8_t red, uint8_t green, uint8_t blue);
void generateFallbackPattern();
void initLedStripColors();
uint32_t complementaryColor(uint32_t color);
uint32_t analogousColor(uint32_t color);
// transmit:
void sendAck();
void sendData();
void sendPacket(String dataToSend);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Setup started");

  // Initialize button input
  pinMode(buttonPin, INPUT);
  Serial.println("Button input initialized");

  // Initialize proximity sensor
  // Wire.begin();
  // if (!proximitySensor.begin(proximitySensorAddress, proximitySensorAddress)) {
  //   Serial.println("Failed to initialize proximity sensor!");
  //   while (1)
  //     ;
  // }
  // Serial.println("Proximity sensor initialized");

  // Initialize LED strip
  strip.begin();
  strip.show();
  Serial.println("LED strip initialized");

  // Initialize LED strip with random colors from an aesthetically-pleasing palette
  initLedStripColors();

  // Connect to WiFi
  connectToWiFi();

  // Start the UDP server
  udp.begin(localPort);
  Serial.print("UDP server started on port ");
  Serial.println(localPort);

  // Create tasks for dual core operation
  xTaskCreatePinnedToCore(coreTask1_LED, "Core1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(coreTask2_WIFI, "Core2", 10000, NULL, 1, NULL, 1);
}


// Connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void coreTask2_WIFI(void* pvParameters) {
  while (1) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      int len = udp.read(packetBuffer, 255);
      if (len > 0) {
        packetBuffer[len] = 0;
      }
      message = String(packetBuffer);
      Serial.println("Contents: ");
      Serial.println(message);

      lastReceivedPacketTime = millis();

      switch (message[0]) {
        case 'W':
          receiveWakeup();
          sendAck();
          break;
        case 'S':
          receiveSleep();
          sendAck();
          break;
        case 'L':
          receiveLEDData();
          sendAck();
          break;
        case 'C':
          receiveConfigData();
          sendAck();
          break;
        default:
          Serial.println("Unknown packet type received.");
      }
    }

    // Send sensor data
    sendData();
    delay(1000);

    // Fallback mode: if no packet received within the fallbackTimeout, generate a pattern based on sensor values
    if (millis() - lastReceivedPacketTime > fallbackTimeout) {
      Serial.println("Entering fallback mode");
      generateFallbackPattern();
    }
  }
}

void receiveWakeup() {
  // Add code to handle wake up packet
  Serial.println("Wakeup packet handled");
}

void receiveSleep() {
  // Add code to handle sleep packet
  Serial.println("Sleep packet handled");
}

void receiveConfigData() {
  // Add code to handle config packet
  Serial.println("Condig packet handled");
}

void receiveLEDData() {
  // Extract color data from the message and update ledColorBuffer
  // Assumes the message format: L<nodeID>,<index1>,<red1>,<green1>,<blue1>,<index2>,<red2>,<green2>,<blue2>,...

  char* ptr = strtok((char*)message.c_str() + 1, ",");

  while (ptr != NULL) {
    int index = atoi(ptr);
    ptr = strtok(NULL, ",");
    if (ptr == NULL) break;
    uint8_t red = atoi(ptr);
    ptr = strtok(NULL, ",");
    if (ptr == NULL) break;
    uint8_t green = atoi(ptr);
    ptr = strtok(NULL, ",");

    if (ptr == NULL) break;
    uint8_t blue = atoi(ptr);
    ptr = strtok(NULL, ",");

    // Update the receiving buffer
    switch (bufferSelect) {
      case 0:
        updateLEDColorBuffer(index, red, green, blue, ledColorBuffer3);
        break;
      case 1:
        updateLEDColorBuffer(index, red, green, blue, ledColorBuffer1);
        break;
      case 2:
        updateLEDColorBuffer(index, red, green, blue, ledColorBuffer2);
        break;
    }
  }

  // Swap buffers
  bufferSelect = (bufferSelect + 1) % 3;
}

void updateLEDColorBuffer(int index, uint8_t red, uint8_t green, uint8_t blue, uint32_t* buffer) {
  buffer[index] = (red << 16) | (green << 8) | blue;
}

void sendAck() {
  String dataToSend = "A" + String(nodeID);
  sendPacket(dataToSend);
}

void sendData() {
  // Read button states
  int buttonState = digitalRead(buttonPin);

  // Read VL53L1X sensor values
  // uint16_t proximity1 = vls3dx_sensor1.read();
  // uint16_t proximity2 = vls3dx_sensor2.read();

  // Create a string with button states and proximity values
  String dataToSend = "D" + String(nodeID) + "," + String(buttonState);  // + "," + String(proximity1) + "," + String(proximity2);
  sendPacket(dataToSend);
}

void sendPacket(String dataToSend) {
  Serial.println("Sending data: " + dataToSend);

  // Send UDP packet
  udp.beginPacket(remoteIP, remotePort);
  udp.write(dataToSend.c_str());
  udp.endPacket();
}



void coreTask1_LED(void* pvParameters) {
  while (1) {
    unsigned long currentTime = millis();
    if (currentTime - lastFrameTime >= 1000 / frameRate) {
      lastFrameTime = currentTime;

      uint32_t* activeBuffer;
      uint32_t* inactiveBuffer;
      uint32_t* receivingBuffer;

      switch (bufferSelect) {
        case 0:
          activeBuffer = ledColorBuffer1;
          inactiveBuffer = ledColorBuffer2;
          receivingBuffer = ledColorBuffer3;
          break;
        case 1:
          activeBuffer = ledColorBuffer2;
          inactiveBuffer = ledColorBuffer3;
          receivingBuffer = ledColorBuffer1;
          break;
        case 2:
          activeBuffer = ledColorBuffer3;
          inactiveBuffer = ledColorBuffer1;
          receivingBuffer = ledColorBuffer2;
          break;
      }

      // Interpolation
      float interpolationFactor = min(1.0, float(currentTime - lastFrameTime) / float(1000 / frameRate));

      for (int i = 0; i < stripLength; i++) {
        uint8_t activeRed = (activeBuffer[i] >> 16) & 0xFF;
        uint8_t activeGreen = (activeBuffer[i] >> 8) & 0xFF;
        uint8_t activeBlue = activeBuffer[i] & 0xFF;

        uint8_t inactiveRed = (inactiveBuffer[i] >> 16) & 0xFF;
        uint8_t inactiveGreen = (inactiveBuffer[i] >> 8) & 0xFF;
        uint8_t inactiveBlue = inactiveBuffer[i] & 0xFF;

        uint8_t interpolatedRed = activeRed * (1 - interpolationFactor) + inactiveRed * interpolationFactor;
        uint8_t interpolatedGreen = activeGreen * (1 - interpolationFactor) + inactiveGreen * interpolationFactor;
        uint8_t interpolatedBlue = activeBlue * (1 - interpolationFactor) + inactiveBlue * interpolationFactor;

        strip.setPixelColor(i, interpolatedRed, interpolatedGreen, interpolatedBlue);
      }

      strip.show();
    }
  }
}







// Color theory utility functions
uint32_t complementaryColor(uint32_t color) {
  uint8_t r = 255 - ((color >> 16) & 0xFF);
  uint8_t g = 255 - ((color >> 8) & 0xFF);
  uint8_t b = 255 - (color & 0xFF);

  return (r << 16) | (g << 8) | b;
}

uint32_t analogousColor(uint32_t color, int angle) {
  // Convert RGB to HSL
  float r = ((color >> 16) & 0xFF) / 255.0;
  float g = ((color >> 8) & 0xFF) / 255.0;
  float b = (color & 0xFF) / 255.0;

  float max_val = max(max(r, g), b);
  float min_val = min(min(r, g), b);
  float h, s, l;

  l = (max_val + min_val) / 2;

  if (max_val == min_val) {
    h = s = 0;
  } else {
    float diff = max_val - min_val;
    s = l > 0.5 ? diff / (2.0 - max_val - min_val) : diff / (max_val + min_val);

    if (max_val == r) {
      h = (g - b) / diff + (g < b ? 6 : 0);
    } else if (max_val == g) {
      h = (b - r) / diff + 2;
    } else {
      h = (r - g) / diff + 4;
    }

    h /= 6;
  }

  // Modify the hue
  h += angle / 360.0;
  while (h < 0) h += 1;
  while (h > 1) h -= 1;

  // Convert back to RGB
  float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
  float p = 2 * l - q;

  auto hueToRgb = [](float p, float q, float t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < 1 / 6.0) return p + (q - p) * 6 * t;
    if (t < 1 / 2.0) return q;
    if (t < 2 / 3.0) return p + (q - p) * (2 / 3.0 - t) * 6;
    return p;
  };

  r = hueToRgb(p, q, h + 1 / 3.0);
  g = hueToRgb(p, q, h);
  b = hueToRgb(p, q, h - 1 / 3.0);

  return (uint32_t(r * 255) << 16) | (uint32_t(g * 255) << 8) | uint32_t(b * 255);
}

// Initialize LED strip with random colors from an aesthetically-pleasing palette
void initLedStripColors() {
  // Define a base color (you can change it to
  // any color you prefer)
  uint32_t baseColor = 0x00FF00;

  // Generate random colors using color theory
  for (int i = 0; i < stripLength; i++) {
    uint32_t color;
    // Randomly select a color mode: 0 = base color, 1 = complementary, 2 = analogous
    int colorMode = random(3);
    switch (colorMode) {
      case 0:
        color = baseColor;
        break;
      case 1:
        color = complementaryColor(baseColor);
        break;
      case 2:
        int angle = random(-30, 31);
        color = analogousColor(baseColor, angle);
        break;
    }

    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Fallback mode timeout (in milliseconds)


// Fallback mode pattern generation
// void generateFallbackPattern() {
//   for (int i = 0; i < stripLength; i++) {
//     // Create a hash of the sensor values
//     int hashValue = (digitalRead(buttonPin) * proximitySensor.read()) % 16777216;
//     uint32_t color = hashValue | (hashValue << 8) | (hashValue << 16);
//     strip.setPixelColor(i, color);
//   }
//   strip.show();
// }
