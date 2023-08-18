#include <ArtnetWifi.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

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
  TRY_WIFI
};

State state = INIT;
int remainingInitFrames = 100;

void setup()
{
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
      strips[i].fill(0xff0000);
      strips[i].show();
    }

    remainingInitFrames--;
    delay(10);

    if (remainingInitFrames == 0)
    {
      for (int i = 0; i < STRIP_COUNT; i++)
      {
        strips[i].fill(0x00ff00);
        strips[i].show();
      }
      state = TRY_WIFI;
    }

    break;
  case TRY_WIFI:
    break;
  }
}
