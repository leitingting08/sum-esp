#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>

// ========== OLED配置 ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI 11
#define OLED_CLK 12
#define OLED_DC 13
#define OLED_CS 14
#define OLED_RESET 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// ========== LED配置 ==========
#define LED_PIN 2
#define NUM_LEDS 30 // 改成你的实际数量

CRGB leds[NUM_LEDS];

// ========== RC522配置 (I2C) ==========
#define I2C_SDA 8
#define I2C_SCL 9
#define RFID_RST 15
#define RFID_ADDR 0x28

// RC522寄存器
#define CommandReg 0x01
#define ComIEnReg 0x02
#define ComIrqReg 0x04
#define ErrorReg 0x06
#define FIFODataReg 0x09
#define FIFOLevelReg 0x0A
#define BitFramingReg 0x0D
#define CollReg 0x0E
#define ModeReg 0x11
#define TxControlReg 0x14
#define TxASKReg 0x15
#define TModeReg 0x2A
#define TPrescalerReg 0x2B
#define TReloadRegH 0x2C
#define TReloadRegL 0x2D
#define VersionReg 0x37

#define PCD_Idle 0x00
#define PCD_Transceive 0x0C
#define PICC_REQIDL 0x26
#define PICC_ANTICOLL 0x93

// ========== RC522函数 ==========
void writeReg(byte reg, byte value)
{
  Wire.beginTransmission(RFID_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

byte readReg(byte reg)
{
  Wire.beginTransmission(RFID_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(RFID_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

void setBitMask(byte reg, byte mask)
{
  writeReg(reg, readReg(reg) | mask);
}

void clearBitMask(byte reg, byte mask)
{
  writeReg(reg, readReg(reg) & (~mask));
}

byte communicate(byte command, byte *sendData, byte sendLen, byte *backData, byte *backLen)
{
  byte irqEn = 0x00;
  byte waitIRq = 0x00;

  if (command == PCD_Transceive)
  {
    irqEn = 0x77;
    waitIRq = 0x30;
  }

  writeReg(ComIEnReg, irqEn | 0x80);
  clearBitMask(ComIrqReg, 0x80);
  setBitMask(FIFOLevelReg, 0x80);
  writeReg(CommandReg, PCD_Idle);

  for (byte i = 0; i < sendLen; i++)
  {
    writeReg(FIFODataReg, sendData[i]);
  }

  writeReg(CommandReg, command);
  if (command == PCD_Transceive)
  {
    setBitMask(BitFramingReg, 0x80);
  }

  int timeout = 2000;
  byte n;
  do
  {
    n = readReg(ComIrqReg);
    timeout--;
  } while ((timeout != 0) && !(n & 0x01) && !(n & waitIRq));

  clearBitMask(BitFramingReg, 0x80);

  if (timeout == 0)
    return 2;

  byte errorReg = readReg(ErrorReg);
  if (errorReg & 0x1B)
    return 1;
  if (n & irqEn & 0x01)
    return 2;

  if (command == PCD_Transceive)
  {
    n = readReg(FIFOLevelReg);
    byte lastBits = readReg(CollReg) & 0x07;
    *backLen = lastBits ? (n - 1) * 8 + lastBits : n * 8;

    if (n > 0)
    {
      for (byte i = 0; i < n; i++)
      {
        backData[i] = readReg(FIFODataReg);
      }
    }
  }
  return 0;
}

byte requestCard(byte *cardType)
{
  byte status, backBits;
  byte buffer[2];

  writeReg(BitFramingReg, 0x07);
  buffer[0] = PICC_REQIDL;

  status = communicate(PCD_Transceive, buffer, 1, buffer, &backBits);

  if (status == 0 && backBits == 0x10)
  {
    *cardType = buffer[0];
    *(cardType + 1) = buffer[1];
    return 0;
  }
  return 1;
}

byte anticoll(byte *uid)
{
  byte status, backBits;
  byte buffer[9];

  writeReg(BitFramingReg, 0x00);
  buffer[0] = PICC_ANTICOLL;
  buffer[1] = 0x20;

  status = communicate(PCD_Transceive, buffer, 2, buffer, &backBits);

  if (status == 0)
  {
    for (byte i = 0; i < 4; i++)
    {
      uid[i] = buffer[i];
    }
  }
  return status;
}

// ========== LED效果函数 ==========
void ledFlash(CRGB color, int times)
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

void ledSolid(CRGB color)
{
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void ledOff()
{
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// ========== 卡片映射 ==========
struct CardInfo
{
  String uid;
  String type;
  String topic;
  String lang;
};

CardInfo cards[10];
int cardCount = 0;

void addCard(String uid, String type, String topic, String lang)
{
  if (cardCount < 10)
  {
    cards[cardCount].uid = uid;
    cards[cardCount].type = type;
    cards[cardCount].topic = topic;
    cards[cardCount].lang = lang;
    cardCount++;
  }
}

CardInfo *findCard(String uid)
{
  for (int i = 0; i < cardCount; i++)
  {
    if (cards[i].uid == uid)
    {
      return &cards[i];
    }
  }
  return nullptr;
}

// ========== 主程序 ==========
void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("  Language Learning Device v1.0");
  Serial.println("========================================\n");

  // 初始化LED
  Serial.print("初始化LED... ");
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  ledFlash(CRGB::Blue, 2); // 启动指示
  Serial.println("✅");

  // 初始化OLED
  Serial.print("初始化OLED... ");
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println("❌");
    ledFlash(CRGB::Red, 5);
    while (1)
      ;
  }
  Serial.println("✅");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // 初始化I2C (RC522)
  Serial.print("初始化I2C... ");
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  delay(100);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(50000);
  Serial.println("✅");

  // 初始化RC522
  Serial.print("初始化RC522... ");
  pinMode(RFID_RST, OUTPUT);
  digitalWrite(RFID_RST, LOW);
  delay(100);
  digitalWrite(RFID_RST, HIGH);
  delay(300);

  byte version = readReg(VersionReg);
  if (version == 0x00 || version == 0xFF)
  {
    Serial.println("❌");
    ledFlash(CRGB::Red, 5);
    while (1)
      ;
  }
  Serial.printf("✅ (v0x%02X)\n", version);

  // 配置RC522
  writeReg(CommandReg, 0x0F);
  delay(50);
  writeReg(TModeReg, 0x80);
  writeReg(TPrescalerReg, 0xA9);
  writeReg(TReloadRegH, 0x03);
  writeReg(TReloadRegL, 0xE8);
  writeReg(TxASKReg, 0x40);
  writeReg(ModeReg, 0x3D);
  setBitMask(TxControlReg, 0x03);

  // 添加卡片映射
  addCard("12:37:64:06", "vocabulary", "daily_routine", "en"); // 蓝色钥匙扣
  addCard("21:47:1f:5d", "grammar", "past_tense", "en");       // 白色卡片
  addCard("88:5a:75:66", "conversation", "restaurant", "es");  // RFID贴纸

  Serial.println("\n✅ 系统就绪！\n");

  // 就绪指示
  ledFlash(CRGB::Green, 2);
  ledOff();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Ready!");
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println("Scan a card to");
  display.println("start learning");
  display.display();
}

void loop()
{
  byte cardType[2];
  byte uid[4];

  if (requestCard(cardType) == 0)
  {
    // 检测到卡片 - 白色闪烁
    ledFlash(CRGB::White, 1);

    if (anticoll(uid) == 0)
    {
      String uidStr = "";
      for (byte i = 0; i < 4; i++)
      {
        if (uid[i] < 0x10)
          uidStr += "0";
        uidStr += String(uid[i], HEX);
        if (i < 3)
          uidStr += ":";
      }

      Serial.println("========================================");
      Serial.print("检测到卡片: ");
      Serial.println(uidStr);

      CardInfo *card = findCard(uidStr);

      if (card != nullptr)
      {
        // 识别成功 - 绿色
        ledSolid(CRGB::Green);

        Serial.print("类型: ");
        Serial.println(card->type);
        Serial.print("主题: ");
        Serial.println(card->topic);
        Serial.print("语言: ");
        Serial.println(card->lang);
        Serial.println("========================================\n");

        delay(500);

        // 学习模式 - 蓝色
        ledSolid(CRGB::Blue);

        // 显示学习内容
        if (card->type == "vocabulary")
        {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.println("Vocabulary");
          display.println("- Daily Routine -");
          display.println("");
          display.setTextSize(2);
          display.println("WAKE UP");
          display.setTextSize(1);
          display.println("");
          display.println("v. 醒来");
          display.println("I wake up at 7am");
          display.display();
        }
        else if (card->type == "grammar")
        {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.println("Grammar");
          display.println("- Past Tense -");
          display.println("");
          display.println("Fill the blank:");
          display.println("");
          display.println("I ___ to school");
          display.println("yesterday.");
          display.println("");
          display.println("A) go  B) went");
          display.display();
        }
        else if (card->type == "conversation")
        {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          display.println("Conversacion");
          display.println("- Restaurante -");
          display.println("");
          display.println("Camarero:");
          display.println("Que desea?");
          display.println("");
          display.println("Tu:");
          display.println("Quiero un cafe");
          display.println("por favor");
          display.display();
        }

        delay(5000);
      }
      else
      {
        // 未知卡片 - 红色闪烁
        ledFlash(CRGB::Red, 3);

        Serial.println("⚠️  未知卡片");
        Serial.println("========================================\n");

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Unknown Card");
        display.println("");
        display.println("UID:");
        display.println(uidStr);
        display.println("");
        display.println("Please register");
        display.display();

        delay(3000);
      }

      // 恢复待机
      ledOff();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("Ready!");
      display.setTextSize(1);
      display.setCursor(0, 25);
      display.println("Scan next card");
      display.display();
    }
  }

  delay(100);
}