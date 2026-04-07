#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI 11  // L13 - D1
#define OLED_CLK 12   // L14 - D0
#define OLED_DC 13    // L15 - DC
#define OLED_CS 14    // L16 - CS
#define OLED_RESET 10 // L12 - RES

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("       OLED 显示屏测试");
    Serial.println("========================================\n");

    Serial.println("接线检查：");
    Serial.println("  GND → 面包板-轨");
    Serial.println("  VCC → 面包板+轨");
    Serial.println("  D0  → ESP32 L14 (GPIO12)");
    Serial.println("  D1  → ESP32 L13 (GPIO11)");
    Serial.println("  RES → ESP32 L12 (GPIO10)");
    Serial.println("  DC  → ESP32 L15 (GPIO13)");
    Serial.println("  CS  → ESP32 L16 (GPIO14)");
    Serial.println("");

    Serial.print("初始化OLED... ");

    if (!display.begin(SSD1306_SWITCHCAPVCC))
    {
        Serial.println("❌ 失败！\n");
        Serial.println("故障排查步骤：");
        Serial.println("1. 用万用表测OLED的VCC引脚");
        Serial.println("   - 黑表笔接-轨");
        Serial.println("   - 红表笔接OLED的VCC");
        Serial.println("   - 应该显示3.3V");
        Serial.println("");
        Serial.println("2. 检查7根线是否插紧");
        Serial.println("");
        Serial.println("3. 尝试重新插拔USB线");
        Serial.println("");
        Serial.println("4. 检查面包板电源轨");
        Serial.println("   - ESP32 L1 → 面包板+轨");
        Serial.println("   - ESP32 L2 → 面包板-轨");

        while (1)
        {
            delay(1000);
        }
    }

    Serial.println("✅ 成功！\n");
    Serial.println("========================================");
    Serial.println("🎉 OLED工作正常！");
    Serial.println("========================================\n");

    // 测试1：显示文本
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED");
    display.println("Works!");
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("Test: OK");
    display.display();

    Serial.println("OLED应该显示：");
    Serial.println("  OLED");
    Serial.println("  Works!");
    Serial.println("  Test: OK");
    Serial.println("");

    delay(3000);

    // 测试2：反色显示
    Serial.println("测试2: 反色显示（3秒后）");
    display.invertDisplay(true);
    delay(3000);
    display.invertDisplay(false);

    Serial.println("测试2: ✅ 完成\n");

    // 测试3：滚动效果
    Serial.println("测试3: 开始滚动测试");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Scroll Test");
    display.println("Watch the screen");
    display.display();

    display.startscrollright(0x00, 0x0F);
    delay(3000);
    display.stopscroll();

    Serial.println("测试3: ✅ 完成\n");

    Serial.println("========================================");
    Serial.println("所有OLED测试通过！");
    Serial.println("========================================\n");
    Serial.println("下一步：测试RC522");
    Serial.println("");
}

void loop()
{
    // 计数器测试
    static int counter = 0;

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("Counter:");
    display.setTextSize(3);
    display.setCursor(20, 25);
    display.print(counter);
    display.display();

    Serial.printf("计数: %d\n", counter);

    counter++;
    delay(1000);
}