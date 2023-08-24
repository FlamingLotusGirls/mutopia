#include <ArtnetWifi.h>
#include <Arduino.h>
#include <NeoPixelBus.h>

IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
const char *ssid = "mutopia";
const char *password = "flgflgflg";

#define STRIP_COUNT 4
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

// NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1X8Ws2812xMethod> strip(])= {
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip1(PIXELS_PER_STRIP, PIN1);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> strip2(PIXELS_PER_STRIP, PIN2);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod> strip3(PIXELS_PER_STRIP, PIN3);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod> strip4(PIXELS_PER_STRIP, PIN4);
// NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip5(PIXELS_PER_STRIP, PIN5);
// NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip6(PIXELS_PER_STRIP, PIN6);
// NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip7(PIXELS_PER_STRIP, PIN7);
// NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip8(PIXELS_PER_STRIP, PIN8);
// };

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
const RgbColor INIT_COLORS[] = {
    RgbColor(0x81, 0x38, 0x0E),
    RgbColor(0xE3, 0x8D, 0x52),
    RgbColor(0x2C, 0xD3, 0xE1),
    RgbColor(0x3C, 0xBD, 0x9F),
    RgbColor(0x00, 0x5A, 0x96),
    RgbColor(0xE8, 0x68, 0x9E),
};

TaskHandle_t WifiTaskHandle;

void setup()
{
  strip1.Begin();
  strip2.Begin();
  strip3.Begin();
  strip4.Begin();
  // strip5.Begin();
  // strip6.Begin();
  // strip7.Begin();
  // strip8.Begin();
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
    strip1.ClearTo(INIT_COLORS[currentInitColor]);
    strip2.ClearTo(INIT_COLORS[currentInitColor]);
    strip3.ClearTo(INIT_COLORS[currentInitColor]);
    strip4.ClearTo(INIT_COLORS[currentInitColor]);
    // strip5.ClearTo(INIT_COLORS[currentInitColor]);
    // strip6.ClearTo(INIT_COLORS[currentInitColor]);
    // strip7.ClearTo(INIT_COLORS[currentInitColor]);
    // strip8.ClearTo(INIT_COLORS[currentInitColor]);
    strip1.Show();
    strip2.Show();
    strip3.Show();
    strip4.Show();
    // strip5.Show();
    // strip6.Show();
    // strip7.Show();
    // strip8.Show();

    delay(500);

    currentInitColor++;
    if (currentInitColor == INIT_COLOR_COUNT)
    {
      // Navigate to TRY_WIFI state
      strip1.ClearTo(INIT_COLORS[4]);
      strip2.ClearTo(INIT_COLORS[4]);
      strip3.ClearTo(INIT_COLORS[4]);
      strip4.ClearTo(INIT_COLORS[4]);
      // strip5.ClearTo(INIT_COLORS[4]);
      // strip6.ClearTo(INIT_COLORS[4]);
      // strip7.ClearTo(INIT_COLORS[4]);
      // strip8.ClearTo(INIT_COLORS[4]);
      strip1.Show();
      strip2.Show();
      strip3.Show();
      strip4.Show();
      // strip5.Show();
      // strip6.Show();
      // strip7.Show();
      // strip8.Show();

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
      strip1.ClearTo(0xE8689E);
      strip2.ClearTo(0xE8689E);
      strip3.ClearTo(0xE8689E);
      strip4.ClearTo(0xE8689E);
      // strip5.ClearTo(0xE8689E);
      // strip6.ClearTo(0xE8689E);
      // strip7.ClearTo(0xE8689E);
      // strip8.ClearTo(0xE8689E);
      strip1.Show();
      strip2.Show();
      strip3.Show();
      strip4.Show();
      // strip5.Show();
      // strip6.Show();
      // strip7.Show();
      // strip8.Show();

      delay(100);

      strip1.ClearTo(0x35081B);
      strip2.ClearTo(0x35081B);
      strip3.ClearTo(0x35081B);
      strip4.ClearTo(0x35081B);
      // strip5.ClearTo(0x35081B);
      // strip6.ClearTo(0x35081B);
      // strip7.ClearTo(0x35081B);
      // strip8.ClearTo(0x35081B);
      strip1.Show();
      strip2.Show();
      strip3.Show();
      strip4.Show();
      // strip5.Show();
      // strip6.Show();
      // strip7.Show();
      // strip8.Show();

      delay(100);
    }

    strip1.ClearTo(0x81380E);
    strip2.ClearTo(0x81380E);
    strip3.ClearTo(0x81380E);
    strip4.ClearTo(0x81380E);
    // strip5.ClearTo(0x81380E);
    // strip6.ClearTo(0x81380E);
    // strip7.ClearTo(0x81380E);
    // strip8.ClearTo(0x81380E);
    strip1.Show();
    strip2.Show();
    strip3.Show();
    strip4.Show();
    // strip5.Show();
    // strip6.Show();
    // strip7.Show();
    // strip8.Show();

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
    strip1.Show();
    strip2.Show();
    strip3.Show();
    strip4.Show();
    // strip5.Show();
    // strip6.Show();
    // strip7.Show();
    // strip8.Show();

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
  uint8_t *pixels;
  switch (stripIndex)
  {
  case 0:
    pixels = strip1.Pixels();
    break;
  case 1:
    pixels = strip2.Pixels();
    break;
  case 2:
    pixels = strip3.Pixels();
    break;
  case 3:
    pixels = strip4.Pixels();
    break;
    // case 4:
    //   pixels = strip5.Pixels();
    //   break;
    // case 5:
    //   pixels = strip6.Pixels();
    //   break;
    // case 6:
    //   pixels = strip7.Pixels();
    //   break;
    // case 7:
    //   pixels = strip8.Pixels();
    //   break;
  }
  memcpy(pixels + (3 * startPixel), data, numPixelsToCopySafely * 3);
  switch (stripIndex)
  {
  case 0:
    strip1.Dirty();
    break;
  case 1:
    strip2.Dirty();
    break;
  case 2:
    strip3.Dirty();
    break;
  case 3:
    strip4.Dirty();
    break;
    // case 4:
    //   strip5.Dirty();
    //   break;
    // case 5:
    //   strip6.Dirty();
    //   break;
    // case 6:
    //   strip7.Dirty();
    //   break;
    // case 7:
    //   strip8.Dirty();
    //   break;
  }
}
