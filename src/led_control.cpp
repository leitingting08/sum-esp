#include "led_control.h"

LEDControl::LEDControl()
{
}

void LEDControl::begin()
{
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(50);
}

void LEDControl::flash(CRGB color, int times)
{
    for (int i = 0; i < times; i++)
    {
        fill_solid(leds, NUM_LEDS, color);
        FastLED.show();
        delay(150);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        delay(150);
    }
}

void LEDControl::solid(CRGB color)
{
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void LEDControl::off()
{
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void LEDControl::setBrightness(uint8_t brightness)
{
    FastLED.setBrightness(brightness);
}

void LEDControl::setCognitiveLoad(float load)
{
    CRGB color;

    if (load < 0.3)
    {
        color = CRGB::Green; // 低负载
    }
    else if (load < 0.7)
    {
        color = CRGB::Yellow; // 中负载
    }
    else
    {
        color = CRGB::Red; // 高负载
    }

    solid(color);
}