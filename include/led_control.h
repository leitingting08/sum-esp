#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <FastLED.h>
#include "config.h"

class LEDControl
{
private:
    CRGB leds[NUM_LEDS];

public:
    LEDControl();

    void begin();
    void flash(CRGB color, int times);
    void solid(CRGB color);
    void off();
    void setBrightness(uint8_t brightness);

    // 根据认知负载设置颜色
    void setCognitiveLoad(float load);
};

#endif