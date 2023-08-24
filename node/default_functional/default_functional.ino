#include <Adafruit_NeoPixel.h>

const int NUM_STRIPS = 8;
// int pins[NUM_STRIPS] = {4, 5, 6, 43, 9, 8, 7, 44};
Adafruit_NeoPixel strips[NUM_STRIPS];
#include <Adafruit_NeoPixel.h>

#define STRIP_LENGTH 720

#define PIN1 4
#define PIN2 5
#define PIN3 6
#define PIN4 43
#define PIN5 9
#define PIN6 8
#define PIN7 7
#define PIN8 44

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(STRIP_LENGTH, PIN1, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(STRIP_LENGTH, PIN2, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(STRIP_LENGTH, PIN3, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(STRIP_LENGTH, PIN4, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(STRIP_LENGTH, PIN5, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip6 = Adafruit_NeoPixel(STRIP_LENGTH, PIN6, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip7 = Adafruit_NeoPixel(STRIP_LENGTH, PIN7, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip8 = Adafruit_NeoPixel(STRIP_LENGTH, PIN8, NEO_RGB + NEO_KHZ800);

void setup()
{
  strip1.begin();
  strip1.setBrightness(50);
  strip1.show();
  strip2.begin();
  strip2.setBrightness(50);
  strip2.show();
  strip3.begin();
  strip3.setBrightness(50);
  strip3.show();
  strip4.begin();
  strip4.setBrightness(50);
  strip4.show();
  strip5.begin();
  strip5.setBrightness(50);
  strip5.show();
  strip6.begin();
  strip6.setBrightness(50);
  strip6.show();
  strip7.begin();
  strip7.setBrightness(50);
  strip7.show();
  strip8.begin();
  strip8.setBrightness(50);
  strip8.show();
}

void loop()
{
  rainbow(20);
}

uint32_t make_pattern(uint32_t i, uint32_t j, uint32_t offset)
{
  float color = i + j + offset;
  if (int(color) % 255 < 100)
  {
    color += 2 * j;
    if (int(color) % 3 == 1)
    {
      color *= -2.5;
    }
  }
  else if (int(color) % 255 > 200)
  {
    if (int(color) % 5 == 0)
    {
      color *= 1.5;
    }
  }
  else
  {
    if (int(color) % 10 == 0)
    { // sparkly
      color += 30;
    }
  }

  return uint32_t(color);
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;
  uint32_t color = 0;
  uint32_t strip_shift = 80;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < STRIP_LENGTH; i++)
    {

      color = Wheel(make_pattern(i, j, 0 * strip_shift));
      strip1.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 1 * strip_shift));
      strip2.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 2 * strip_shift));
      strip3.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 3 * strip_shift));
      strip4.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 4 * strip_shift));
      strip5.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 5 * strip_shift));
      strip6.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 6 * strip_shift));
      strip7.setPixelColor(i, color);
      color = Wheel(make_pattern(i, j, 7 * strip_shift));
      strip8.setPixelColor(i, color);
    }
    strip1.show();
    strip2.show();
    strip3.show();
    strip4.show();
    strip5.show();
    strip6.show();
    strip7.show();
    strip8.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
