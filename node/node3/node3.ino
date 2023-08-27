#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>

IPAddress ip(192, 168, 0, 201);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
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
TaskHandle_t RenderTaskHandle;

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
  xTaskCreatePinnedToCore(renderTask, "RenderTask", 10000, nullptr, 20 /* priority */, &RenderTaskHandle, 1 /* core */);
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

void renderTask(void *params)
{
  Serial.print("renderTask, on core ");
  Serial.println(xPortGetCoreID());

  while (true)
  {
    loopRenderTask();
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
      if (++dots == 10)
      {
        dots = 0;
        Serial.println("");
        Serial.print("Status: ");
        switch (WiFi.status())
        {
        case WL_NO_SHIELD:
          Serial.println("WL_NO_SHIELD");
          break;
        case WL_IDLE_STATUS:
          Serial.println("WL_IDLE_STATUS");
          break;
        case WL_NO_SSID_AVAIL:
          Serial.println("WL_NO_SSID_AVAIL");
          break;
        case WL_SCAN_COMPLETED:
          Serial.println("WL_SCAN_COMPLETED");
          break;
        case WL_CONNECTED:
          Serial.println("WL_CONNECTED");
          break;
        case WL_CONNECT_FAILED:
          Serial.println("WL_CONNECT_FAILED");
          break;
        case WL_CONNECTION_LOST:
          Serial.println("WL_CONNECTION_LOST");
          break;
        case WL_DISCONNECTED:
          Serial.println("WL_DISCONNECTED");
          break;
        }
        beginWifi();
      }

      for (int i = 0; i < 25; i++)
      {
        renderIdlePattern();
        delay(8);
      }
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
}

void loopRenderTask()
{
  switch (state)
  {
  case RECEIVE_ARTNET:
    FastLED.show();
    break;
  }
  vTaskDelay(1);
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

#define MIN_HUE_NORM (1 / 6)
#define MAX_HUE_NORM 1
#define HUE_SPREAD_NORM (MAX_HUE_NORM - MIN_HUE_NORM) / 2
#define HUE_CENTER_NORM (MIN_HUE_NORM + HUE_SPREAD_NORM)
const float hueCenterU16 = HUE_CENTER_NORM * 65536;
const float hueSpreadU16 = HUE_SPREAD_NORM * 65536;

void renderIdlePattern()
{
  int deciseconds = millis() / 100;
  for (int stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++)
  {
    Adafruit_NeoPixel &strip = strips[stripIndex];
    int stripNumPixels = strip.numPixels();
    for (int pixelIndex = 0; pixelIndex < stripNumPixels; pixelIndex++)
    {
      int adjustedPixelIndex = pixelIndex + 100 * stripIndex;
      // noise is -1 to 1
      float hueNoise = PerlinNoise2(deciseconds /* x */, adjustedPixelIndex /* y */, 0.25 /* persistence */, 3 /* octaves */);
      float satNoise = PerlinNoise2(deciseconds /* x */, adjustedPixelIndex / 10 /* y */, 0.25 /* persistence */, 3 /* octaves */);
      float valNoise = PerlinNoise2(deciseconds /* x */, adjustedPixelIndex / 13 /* y */, 0.25 /* persistence */, 3 /* octaves */);

      float hueU16 = hueNoise * hueSpreadU16 + hueCenterU16;
      float satU8 = satNoise * 75 + 180;
      float valU8 = valNoise * 75 + 180;

      strip.setPixelColor(pixelIndex, Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(hueU16, satU8, valU8)));
    }
    strip.show();
  }
}

// // PERLIN NOISE // //

// using the algorithm from http://freespace.virgin.net/hugo.elias/models/m_perlin.html
//  thanks to hugo elias
float Noise2(float x, float y)
{
  long noise;
  noise = x + y * 57;
  noise = pow(noise << 13, noise);
  return (1.0 - (long(noise * (noise * noise * 15731L + 789221L) + 1376312589L) & 0x7fffffff) / 1073741824.0);
}

float SmoothNoise2(float x, float y)
{
  float corners, sides, center;
  corners = (Noise2(x - 1, y - 1) + Noise2(x + 1, y - 1) + Noise2(x - 1, y + 1) + Noise2(x + 1, y + 1)) / 16;
  sides = (Noise2(x - 1, y) + Noise2(x + 1, y) + Noise2(x, y - 1) + Noise2(x, y + 1)) / 8;
  center = Noise2(x, y) / 4;
  return (corners + sides + center);
}

float InterpolatedNoise2(float x, float y)
{
  float v1, v2, v3, v4, i1, i2, fractionX, fractionY;
  long longX, longY;

  longX = long(x);
  fractionX = x - longX;

  longY = long(y);
  fractionY = y - longY;

  v1 = SmoothNoise2(longX, longY);
  v2 = SmoothNoise2(longX + 1, longY);
  v3 = SmoothNoise2(longX, longY + 1);
  v4 = SmoothNoise2(longX + 1, longY + 1);

  i1 = Interpolate(v1, v2, fractionX);
  i2 = Interpolate(v3, v4, fractionX);

  return (Interpolate(i1, i2, fractionY));
}

float Interpolate(float a, float b, float x)
{
  // cosine interpolations
  return (CosineInterpolate(a, b, x));
}

float LinearInterpolate(float a, float b, float x)
{
  return (a * (1 - x) + b * x);
}

float CosineInterpolate(float a, float b, float x)
{
  float ft = x * 3.1415927;
  float f = (1 - cos(ft)) * .5;

  return (a * (1 - f) + b * f);
}

float PerlinNoise2(float x, float y, float persistence, int octaves)
{
  float frequency, amplitude;
  float total = 0.0;

  for (int i = 0; i <= octaves - 1; i++)
  {
    frequency = pow(2, i);
    amplitude = pow(persistence, i);

    total = total + InterpolatedNoise2(x * frequency, y * frequency) * amplitude;
  }

  return (total);
}

// // END PERLIN NOISE // //