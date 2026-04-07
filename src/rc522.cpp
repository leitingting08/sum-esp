#include <Arduino.h>
#include <Wire.h>
#include <MFRC522_I2C.h>

// I2C配置
#define I2C_SDA 8
#define I2C_SCL 9
#define RFID_RST 15
#define RFID_ADDR 0x28 // 根据你扫描到的地址修改（可能是0x28或0x2C）

// 创建MFRC522实例
MFRC522_I2C mfrc522(RFID_ADDR, RFID_RST);

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("  RC522 RFID读卡测试 (I2C)");
    Serial.println("========================================\n");

    // 初始化I2C
    pinMode(I2C_SDA, INPUT_PULLUP);
    pinMode(I2C_SCL, INPUT_PULLUP);
    delay(50);

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000); // 100kHz

    Serial.println("I2C初始化完成");

    // 初始化MFRC522
    mfrc522.PCD_Init();
    delay(100);

    // 读取版本信息
    byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    Serial.printf("MFRC522版本: 0x%02X\n", version);

    if (version == 0x91 || version == 0x92)
    {
        Serial.println("✅ RC522初始化成功！\n");
    }
    else
    {
        Serial.printf("⚠️  版本号异常: 0x%02X\n", version);
        Serial.println("   但可以尝试继续...\n");
    }

    Serial.println("========================================");
    Serial.println("请将RFID卡片靠近读卡器（2-3cm内）");
    Serial.println("========================================\n");
}

void loop()
{
    // 检测是否有新卡片
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        delay(50);
        return;
    }

    // 读取卡片序列号
    if (!mfrc522.PICC_ReadCardSerial())
    {
        delay(50);
        return;
    }

    // 成功读取到卡片
    Serial.println("========================================");
    Serial.println("✅ 检测到RFID卡片！");
    Serial.println("========================================");

    // 显示UID
    Serial.print("卡片UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        if (mfrc522.uid.uidByte[i] < 0x10)
        {
            Serial.print("0");
        }
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1)
        {
            Serial.print(":");
        }
    }
    Serial.println();

    // 显示卡片类型
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.print("卡片类型: ");
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    Serial.println("========================================\n");

    // 停止读卡
    mfrc522.PICC_HaltA();

    delay(2000); // 2秒后可以再次读卡
}