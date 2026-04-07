#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 2 // ← 改成2
#define NUM_LEDS 30

CRGB leds[NUM_LEDS];

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("  WS2812B灯带测试");
    Serial.println("========================================\n");

    Serial.printf("LED数量: %d\n", NUM_LEDS);
    Serial.println("接线：");
    Serial.println("  +5V (红) → ESP32 5V");
    Serial.println("  DIN (绿) → ESP32 GPIO2"); // ← 改成GPIO2
    Serial.println("  GND (白) → ESP32 GND\n");

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(50);

    Serial.println("初始化完成，开始测试...\n");
}

void loop()
{
    Serial.println("红色");
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    delay(1000);

    Serial.println("绿色");
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(1000);

    Serial.println("蓝色");
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
    delay(1000);

    Serial.println("关闭");
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(1000);

    Serial.println("--- 循环 ---\n");
}