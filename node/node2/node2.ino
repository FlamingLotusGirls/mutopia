/*
This example will receive multiple universes via Artnet and control a strip of ws2811 strip via
Adafruit's NeoPixel library: https://github.com/adafruit/Adafruit_NeoPixel
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Wifi settings
const char *ssid = "TP-Link_81DC";
const char *password = "18585223";

// Neopixel settings
const int numPixels = 30;
const int numberOfChannels = numPixels * 3; // Total number of channels you want to receive (1 led = 3 channels)
const byte dataPin = 4;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, dataPin, NEO_GRB + NEO_KHZ800);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;
bool connectionSuccessful = false;

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 20)
    {
      state = false;
      break;
    }
    i++;
  }
  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    connectionSuccessful = true;
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
    connectionSuccessful = false;
  }

  return state;
}

void initTest()
{
  for (int i = 0; i < numPixels; i++)
    strip.setPixelColor(i, 127, 0, 0);
  strip.show();
  delay(500);
  for (int i = 0; i < numPixels; i++)
    strip.setPixelColor(i, 0, 127, 0);
  strip.show();
  delay(500);
  for (int i = 0; i < numPixels; i++)
    strip.setPixelColor(i, 0, 0, 127);
  strip.show();
  delay(500);
  for (int i = 0; i < numPixels; i++)
    strip.setPixelColor(i, 0, 0, 0);
  strip.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  // Debug output
  Serial.print("DMX: Univ: ");
  Serial.print(universe, DEC);
  Serial.print(", Seq: ");
  Serial.print(sequence, DEC);
  Serial.print(", Data (");
  Serial.print(length, DEC);
  Serial.print("): ");
  bool tail = false;
  uint16_t printLength = length;
  if (length > 16)
  {
    printLength = 16;
    tail = true;
  }
  // Print the buffer
  for (int i = 0; i < printLength; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  if (tail)
  {
    Serial.print("...");
  }
  Serial.println();

  // Show LEDs

  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15)
  {
    strip.setBrightness(data[0]);
    strip.show();
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++)
  {
    if (universesReceived[i] == 0)
    {
      Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numPixels)
      strip.setPixelColor(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  previousDataLength = length;

  if (sendFrame)
  {
    strip.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setup()
{
  Serial.begin(115200);
  while (!ConnectWifi())
  {
    for (int j = 0; j < 5; j++)
    {
      for (int i = 0; i < numPixels; i++)
        strip.setPixelColor(i, 127, 0, 0);
      strip.show();
      delay(100);
      for (int i = 0; i < numPixels; i++)
        strip.setPixelColor(i, 0, 0, 0);
      strip.show();
      delay(100);
    }
  }
  artnet.begin();
  strip.begin();
  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  Serial.println("Setup done");
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
