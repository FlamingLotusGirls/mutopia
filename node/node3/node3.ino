#include <ArtnetWifi.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
const char *ssid = "mutopia";
const char *password = "flgflgflg";

#define STRIP_COUNT 8
#define STRIP_LENGTH 720
#define UNIV_PERIOD 10
#define PIXELS_PER_UNIV 170

struct Strip
{
  Adafruit_NeoPixel pixels;
  Strip(int len, int pin, int flags) : pixels(len, pin, flags) {}
};

#define PIN1 4
#define PIN2 5
#define PIN3 6
#define PIN4 43
#define PIN5 9
#define PIN6 8
#define PIN7 7
#define PIN8 44

Strip strips[] = {
    Strip(STRIP_LENGTH, PIN1, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN2, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN3, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN4, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN5, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN6, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN7, NEO_RGB + NEO_KHZ800),
    Strip(STRIP_LENGTH, PIN8, NEO_RGB + NEO_KHZ800)};

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

void setup()
{
  Serial.begin(115200);
  Serial.println("");
  Serial.println("FLG Mutopia LED Node");
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    strips[i].pixels.begin();
  }
}

void beginWifi()
{
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
}

void loop()
{
  int dots = 0;
  switch (state)
  {
  case INIT:
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      strips[i].pixels.fill(INIT_COLORS[currentInitColor]);
      strips[i].pixels.show();
    }

    delay(500);

    currentInitColor++;
    if (currentInitColor == INIT_COLOR_COUNT)
    {
      // Navigate to TRY_WIFI state
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        strips[i].pixels.fill(INIT_COLORS[4]);
        strips[i].pixels.show();
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
        strips[i].pixels.fill(0xE8689E);
        strips[i].pixels.show();
      }
      delay(100);
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        strips[i].pixels.fill(0x35081B);
        strips[i].pixels.show();
      }
      delay(100);
    }
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      strips[i].pixels.fill(0x81380E);
      strips[i].pixels.show();
    }
    delay(100);

    // Navigate to RECEIVE_ARTNET state
    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
    state = RECEIVE_ARTNET;
    break;
  case RECEIVE_ARTNET:
    artnet.read();
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
    Serial.println("stripIndex too high");
    return;
  }
  int stripUniverse = universe % UNIV_PERIOD;
  int startPixel = stripUniverse * PIXELS_PER_UNIV;
  if (startPixel >= STRIP_LENGTH)
  {
    Serial.println("startPixel too high");
    return;
  }

  Strip &strip = strips[stripIndex];

  // MAYBE SHOW
  // Simply show if we receive a 0 universe, that way on average we will show once per strip.
  // This way of deciding when to show may seem naive, but upon testing, perceptually, it performs
  // as well or better than a few smarter algorithms (while smarter algorithms can be more sure
  // that the same frame data is being shown at the same time, it often comes at the cost of frame
  // drops). This way also just has fewer failure cases.
  // We show before storing the new pixel data because, if we receive frames in the right order,
  // each 0 universe will mean we are done receiving the previous frame's data.
  if (stripUniverse == 0)
  {
    strip.pixels.show();
  }

  // STORE ARTNET DATA INTO PIXEL COLORS
  // Sometimes the received length isn't a multiple of 3 (e.g. Chromatik seems to always send an
  // even length and pad the buffer with a zero if pixel count is odd).
  int numPixelsReceivedThisFrame = length / 3;

  for (size_t receivedPixelIndex = 0;
       receivedPixelIndex < numPixelsReceivedThisFrame;
       receivedPixelIndex++)
  {
    size_t pixelStartInData = receivedPixelIndex * 3;
    strip.pixels.setPixelColor(
        startPixel + receivedPixelIndex,
        data[pixelStartInData],
        data[pixelStartInData + 1],
        data[pixelStartInData + 2]);
  }
}
