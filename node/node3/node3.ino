#include <ArtnetWifi.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

IPAddress ip(192, 168, 0, 100);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
const char *ssid = "mutopia";
const char *password = "flgflgflg";

#define STRIP_COUNT 8
#define STRIP_LENGTH 180

#define PIN1 4
#define PIN2 5
#define PIN3 6
#define PIN4 43
#define PIN5 9
#define PIN6 8
#define PIN7 7
#define PIN8 44

Adafruit_NeoPixel strips[] = {
    Adafruit_NeoPixel(STRIP_LENGTH, PIN1, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN2, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN3, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN4, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN5, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN6, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN7, NEO_RGB + NEO_KHZ800),
    Adafruit_NeoPixel(STRIP_LENGTH, PIN8, NEO_RGB + NEO_KHZ800)};

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
    strips[i].begin();
  }
}

void loop()
{
  switch (state)
  {
  case INIT:
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      strips[i].fill(INIT_COLORS[currentInitColor]);
      strips[i].show();
    }

    delay(500);

    currentInitColor++;
    if (currentInitColor == INIT_COLOR_COUNT)
    {
      // Navigate to TRY_WIFI state
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        strips[i].fill(INIT_COLORS[4]);
        strips[i].show();
      }
      state = TRY_WIFI;
    }

    break;
  case TRY_WIFI:
    Serial.print("Searching for WiFi");
    WiFi.begin(ssid, password);
    WiFi.config(ip, gateway, subnet);

    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
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
        strips[i].fill(0xE8689E);
        strips[i].show();
      }
      delay(100);
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        strips[i].fill(0x35081B);
        strips[i].show();
      }
      delay(100);
    }
    for (int i = 0; i < STRIP_COUNT; i++)
    {
      strips[i].fill(0x81380E);
      strips[i].show();
    }
    delay(100);

    // Navigate to RECEIVE_ARTNET state
    state = RECEIVE_ARTNET;
    break;
  case RECEIVE_ARTNET:
    break;
  }
}
