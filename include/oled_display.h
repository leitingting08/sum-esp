#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_SSD1306.h>
#include "config.h"

class OLEDDisplay
{
private:
    Adafruit_SSD1306 *display;

public:
    OLEDDisplay();
    ~OLEDDisplay();

    bool begin();
    void clear();

    // 系统界面
    void showStartup();
    void showReady();

    // 学习内容显示
    void showVocabulary(String word, String translation, String example);
    void showGrammar(String title, String question, String optionA, String optionB);
    void showConversation(String title, String line1, String line2, String line3, String line4);
    void showUnknownCard(String uid);

    // 传感器状态显示
    void showSensorStatus(float gsr, float fidget, int hr);
};

#endif