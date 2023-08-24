#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>

IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
const char *ssid = "mutopia";
const char *password = "flgflgflg";

#define STRIP_COUNT 8
#define UNIV_PERIOD 10
#define PIXELS_PER_STRIP 720
#define PIXELS_PER_UNIV 170
#define BRIGHTNESS 255

#define PIN1 4
#define PIN2 5
#define PIN3 6
#define PIN4 43
#define PIN5 9
#define PIN6 8
#define PIN7 7
#define PIN8 44

CRGB strips[STRIP_COUNT][PIXELS_PER_STRIP];

ArtnetWifi artnet;

enum State
{
  INIT,
  TRY_WIFI,
  RECEIVE_ARTNET
};

State state = INIT;
int currentInitColor = 0;

const int INIT_COLOR_COUNT = 6;
const int INIT_COLORS[] = {
    0x81380E,
    0xE38D52,
    0x2CD3E1,
    0x3CBD9F,
    0x005A96,
    0xE8689E,
};

TaskHandle_t WifiTaskHandle;

void setup()
{
  FastLED.addLeds<WS2812, PIN1>(strips[0], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN2>(strips[1], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN3>(strips[2], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN4>(strips[3], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN5>(strips[4], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN6>(strips[5], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN7>(strips[6], PIXELS_PER_STRIP);
  FastLED.addLeds<WS2812, PIN8>(strips[7], PIXELS_PER_STRIP);

  Serial.begin(115200);
  Serial.println("");
  Serial.println("FLG Mutopia LED Node");

  Serial.print("setup, on core ");
  Serial.println(xPortGetCoreID());

  // Wifi normally runs on core 0, so we are using core 0 to make sure our reading of packets is
  // synchronous with the ESP32 infra's code that receives packets.
  xTaskCreatePinnedToCore(wifiTask, "WifiTask", 10000, nullptr, 1 /* priority */, &WifiTaskHandle, 0 /* core */);
}

void wifiTask(void *params)
{
  Serial.print("wifiTask, on core ");
  Serial.println(xPortGetCoreID());

  while (true)
  {
    loopWifiTask();
  }
}

void beginWifi()
{
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
}

void loopWifiTask()
{
  int dots = 0;
  switch (state)
  {
  case INIT:
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      fill_solid(strips[i], PIXELS_PER_STRIP, INIT_COLORS[currentInitColor]);
      FastLED[i].showLeds(BRIGHTNESS);
    }

    delay(500);

    currentInitColor++;
    if (currentInitColor == INIT_COLOR_COUNT)
    {
      // Navigate to TRY_WIFI state
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        fill_solid(strips[i], PIXELS_PER_STRIP, INIT_COLORS[4]);
        FastLED[i].showLeds(BRIGHTNESS);
      }
      state = TRY_WIFI;
    }

    break;
  case TRY_WIFI:
    Serial.print("Searching for WiFi");
    beginWifi();

    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      if (++dots == 20)
      {
        dots = 0;
        Serial.println("");
        Serial.print("Status: ");
        Serial.println(WiFi.status());
        beginWifi();
      }
      delay(200);
    }
    Serial.println("");
    Serial.print("Connected to \"");
    Serial.print(ssid);
    Serial.println("\"");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI Strength: ");
    Serial.println(WiFi.RSSI());

    // Confirm WiFi connected with pink LED flashes
    for (int j = 3; j > 0; j--)
    {
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        fill_solid(strips[i], PIXELS_PER_STRIP, 0xE8689E);
        FastLED[i].showLeds(BRIGHTNESS);
      }
      delay(100);
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        fill_solid(strips[i], PIXELS_PER_STRIP, 0x35081B);
        FastLED[i].showLeds(BRIGHTNESS);
      }
      delay(100);
    }
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      fill_solid(strips[i], PIXELS_PER_STRIP, 0x81380E);
      FastLED[i].showLeds(BRIGHTNESS);
    }
    delay(100);

    // Navigate to RECEIVE_ARTNET state
    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
    state = RECEIVE_ARTNET;
    break;
  case RECEIVE_ARTNET:
    artnet.read();
    vTaskDelay(1);
    break;
  }
}

void loop()
{
  switch (state)
  {
  case RECEIVE_ARTNET:
    // for (int i = 0; i < STRIP_COUNT; i++)
    // {
    FastLED.show();
    // }

    break;
  }
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  // Serial.print("DMX: Univ: ");
  // Serial.print(universe, DEC);
  // Serial.print(", Seq: ");
  // Serial.print(sequence, DEC);
  // Serial.print(", Length: ");
  // Serial.print(length, DEC);
  // Serial.print("; ");
  // bool limitPrintLength = false;
  // uint16_t printLength = length;
  // if (length > 16)
  // {
  //   printLength = 16;
  //   limitPrintLength = true;
  // }
  // // Print the buffer
  // for (int i = 0; i < printLength; i++)
  // {
  //   Serial.print(data[i], HEX);
  //   Serial.print(" ");
  // }
  // if (limitPrintLength)
  // {
  //   Serial.print("...");
  // }
  // Serial.println();

  // FIGURE OUT WHICH STRIP & WHETHER WE HAVE PIXELS THAT CAN DISPLAY THIS FRAME
  int stripIndex = universe / UNIV_PERIOD;
  if (stripIndex >= STRIP_COUNT)
  {
    return;
  }
  int stripUniverse = universe % UNIV_PERIOD;
  int startPixel = stripUniverse * PIXELS_PER_UNIV;
  if (startPixel >= PIXELS_PER_STRIP)
  {
    return;
  }

  // STORE ARTNET DATA INTO PIXEL COLORS
  // Sometimes the received length isn't a multiple of 3 (e.g. Chromatik seems to always send an
  // even length and pad the buffer with a zero if pixel count is odd).
  int numPixelsReceivedThisFrame = length / 3;

  int numPixelsToCopySafely = min(PIXELS_PER_STRIP - startPixel, numPixelsReceivedThisFrame);
  memcpy(strips[stripIndex] + startPixel, data, numPixelsToCopySafely * 3);
}
