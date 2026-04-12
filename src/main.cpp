#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "oled_display.h"
#include "led_control.h"
#include "gsr_sensor.h"
#include "motion_sensor.h"
// #include "heart_sensor.h"  // ← 注释掉

// 创建传感器对象
OLEDDisplay oled;
LEDControl led;
GSRSensor gsr;
MotionSensor motion(MOTION_SENSOR_ADDR);
// HeartSensor heart(HEART_SENSOR_ADDR);  // ← 注释掉

// 学习内容类型
enum ContentType
{
  VOCABULARY,
  GRAMMAR,
  CONVERSATION
};

ContentType currentContent = VOCABULARY;
unsigned long lastContentChange = 0;
const unsigned long CONTENT_INTERVAL = 10000;

// 传感器状态
bool gsrReady = false;
bool motionReady = false;
// bool heartReady = false;  // ← 注释掉

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(2000);

  Serial.println("\n========================================");
  Serial.println("  Language Learning Device v2.0");
  Serial.println("  认知负载检测系统");
  Serial.println("========================================\n");

  // 初始化I2C总线
  Serial.print("初始化I2C总线... ");
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  delay(100);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(I2C_SPEED);
  Serial.println("✅");

  // 初始化LED
  Serial.print("初始化LED... ");
  led.begin();
  led.flash(CRGB::Blue, 2);
  Serial.println("✅");

  // 初始化OLED
  Serial.print("初始化OLED... ");
  if (!oled.begin())
  {
    Serial.println("❌");
    led.flash(CRGB::Red, 5);
    while (1)
      ;
  }
  Serial.println("✅");
  oled.showStartup();
  delay(1000);

  // 初始化GSR
  Serial.print("初始化GSR... ");
  gsr.begin();
  Serial.println("✅");

  // 初始化运动传感器
  Serial.print("初始化运动传感器... ");
  if (!motion.begin())
  {
    Serial.println("❌ (继续运行)");
  }
  else
  {
    Serial.println("✅");
    motionReady = true;
  }

  // ← 注释掉心率传感器初始化

  Serial.println("\n✅ 系统就绪！\n");

  led.flash(CRGB::Green, 2);
  led.off();

  // 校准传感器
  Serial.println("========================================");
  Serial.println("  传感器校准");
  Serial.println("========================================\n");

  Serial.println("GSR校准:");
  Serial.println("  请将电极夹在手指上");
  Serial.println("  保持放松状态");
  delay(3000);
  gsr.calibrate(10);
  gsrReady = true;

  if (motionReady)
  {
    Serial.println("\n运动传感器校准:");
    Serial.println("  请保持设备静止");
    delay(2000);
    motion.calibrateFidgeting(10);
  }

  Serial.println("\n========================================");
  Serial.println("  开始学习模式");
  Serial.println("========================================\n");

  oled.showReady();
  delay(2000);
}

void loop()
{
  unsigned long currentTime = millis();

  // 自动切换内容
  if (currentTime - lastContentChange > CONTENT_INTERVAL)
  {
    lastContentChange = currentTime;

    currentContent = (ContentType)((currentContent + 1) % 3);

    Serial.println("\n========================================");
    Serial.print("切换内容: ");

    switch (currentContent)
    {
    case VOCABULARY:
      Serial.println("Vocabulary");
      oled.showVocabulary("STUDY", "v. 学习", "I study English");
      break;

    case GRAMMAR:
      Serial.println("Grammar");
      oled.showGrammar("Present Tense",
                       "She ___ to school\nevery day.",
                       "go", "goes");
      break;

    case CONVERSATION:
      Serial.println("Conversation");
      oled.showConversation("Greeting",
                            "A: Hello!",
                            "B: Hi! How are you?",
                            "A: I'm fine, thanks.",
                            "B: Nice to meet you!");
      break;
    }

    led.flash(CRGB::White, 1);
  }

  // 读取传感器
  float gsrLoad = 0.0;
  float motionLoad = 0.0;

  if (gsrReady)
  {
    gsrLoad = gsr.getCognitiveLoad();
  }

  if (motionReady)
  {
    motionLoad = motion.getFidgetingLevel();
  }

  // 融合认知负载（只用GSR和运动）
  float totalLoad = 0.0;

  if (gsrReady)
  {
    totalLoad += gsrLoad * 0.7; // GSR权重70%
  }

  if (motionReady)
  {
    totalLoad += motionLoad * 0.3; // 运动权重30%
  }

  // LED反馈
  led.setCognitiveLoad(totalLoad);

  // 串口输出
  static unsigned long lastPrint = 0;
  if (currentTime - lastPrint > 2000)
  {
    lastPrint = currentTime;

    Serial.println("----------------------------------------");
    Serial.println("传感器数据:");

    if (gsrReady)
    {
      Serial.printf("  GSR:    %.2f V (负载: %.2f)\n",
                    gsr.getVoltage(), gsrLoad);
    }

    if (motionReady)
    {
      int16_t ax, ay, az;
      motion.getAccel(ax, ay, az);
      Serial.printf("  运动:   加速度(%d,%d,%d) (负载: %.2f)\n",
                    ax, ay, az, motionLoad);
    }

    Serial.printf("\n总体认知负载: %.2f ", totalLoad);

    if (totalLoad < LOAD_LOW_THRESHOLD)
    {
      Serial.println("(舒适) 🟢");
    }
    else if (totalLoad < LOAD_HIGH_THRESHOLD)
    {
      Serial.println("(适中) 🟡");
    }
    else
    {
      Serial.println("(压力) 🔴");
    }

    Serial.println("----------------------------------------\n");
  }

  delay(100);
}