#include "oled_display.h"
#include <SPI.h>

OLEDDisplay::OLEDDisplay()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT,
                                   OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
}

OLEDDisplay::~OLEDDisplay()
{
    delete display;
}

bool OLEDDisplay::begin()
{
    if (!display->begin(SSD1306_SWITCHCAPVCC))
    {
        return false;
    }
    display->setTextColor(SSD1306_WHITE);
    clear();
    return true;
}

void OLEDDisplay::clear()
{
    display->clearDisplay();
    display->display();
}

void OLEDDisplay::showStartup()
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Initializing...");
    display->display();
}

void OLEDDisplay::showReady()
{
    display->clearDisplay();
    display->setTextSize(2);
    display->setCursor(0, 0);
    display->println("Ready!");
    display->setTextSize(1);
    display->setCursor(0, 25);
    display->println("Scan a card to");
    display->println("start learning");
    display->display();
}

void OLEDDisplay::showVocabulary(String word, String translation, String example)
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Vocabulary");
    display->println("- Daily Routine -");
    display->println("");

    display->setTextSize(2);
    display->println(word);

    display->setTextSize(1);
    display->println("");
    display->println(translation);
    display->println(example);
    display->display();
}

void OLEDDisplay::showGrammar(String title, String question, String optionA, String optionB)
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Grammar");
    display->print("- ");
    display->print(title);
    display->println(" -");
    display->println("");
    display->println(question);
    display->println("");
    display->print("A) ");
    display->print(optionA);
    display->print("  B) ");
    display->println(optionB);
    display->display();
}

void OLEDDisplay::showConversation(String title, String line1, String line2, String line3, String line4)
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Conversacion");
    display->print("- ");
    display->print(title);
    display->println(" -");
    display->println("");
    display->println(line1);
    display->println(line2);
    display->println("");
    display->println(line3);
    display->println(line4);
    display->display();
}

void OLEDDisplay::showUnknownCard(String uid)
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Unknown Card");
    display->println("");
    display->println("UID:");
    display->println(uid);
    display->println("");
    display->println("Please register");
    display->display();
}

void OLEDDisplay::showSensorStatus(float gsr, float fidget, int hr)
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->println("Sensor Status");
    display->println("-------------");

    display->print("GSR:    ");
    display->println(gsr, 2);

    display->print("Fidget: ");
    display->println(fidget, 2);

    display->print("HR:     ");
    display->print(hr);
    display->println(" bpm");

    display->display();
}