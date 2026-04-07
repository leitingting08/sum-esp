#include <Arduino.h>
#include <Wire.h>

// I2C配置
#define I2C_SDA 8
#define I2C_SCL 9
#define RFID_RST 15
#define RFID_ADDR 0x28 // 你扫描到的地址

// MFRC522寄存器
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

// MFRC522命令
#define PCD_Idle 0x00
#define PCD_Transceive 0x0C
#define PICC_REQIDL 0x26
#define PICC_ANTICOLL 0x93

// 写寄存器
void writeReg(byte reg, byte value)
{
    Wire.beginTransmission(RFID_ADDR);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

// 读寄存器
byte readReg(byte reg)
{
    Wire.beginTransmission(RFID_ADDR);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(RFID_ADDR, 1);
    return Wire.available() ? Wire.read() : 0;
}

// 设置寄存器位
void setBitMask(byte reg, byte mask)
{
    byte tmp = readReg(reg);
    writeReg(reg, tmp | mask);
}

// 清除寄存器位
void clearBitMask(byte reg, byte mask)
{
    byte tmp = readReg(reg);
    writeReg(reg, tmp & (~mask));
}

// 与卡片通信
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

    // 写数据到FIFO
    for (byte i = 0; i < sendLen; i++)
    {
        writeReg(FIFODataReg, sendData[i]);
    }

    // 执行命令
    writeReg(CommandReg, command);

    if (command == PCD_Transceive)
    {
        setBitMask(BitFramingReg, 0x80);
    }

    // 等待完成
    int timeout = 2000;
    byte n;
    do
    {
        n = readReg(ComIrqReg);
        timeout--;
    } while ((timeout != 0) && !(n & 0x01) && !(n & waitIRq));

    clearBitMask(BitFramingReg, 0x80);

    if (timeout == 0)
    {
        return 2; // 超时
    }

    byte errorReg = readReg(ErrorReg);
    if (errorReg & 0x1B)
    {
        return 1; // 错误
    }

    if (n & irqEn & 0x01)
    {
        return 2; // 超时
    }

    if (command == PCD_Transceive)
    {
        n = readReg(FIFOLevelReg);
        byte lastBits = readReg(CollReg) & 0x07;

        if (lastBits)
        {
            *backLen = (n - 1) * 8 + lastBits;
        }
        else
        {
            *backLen = n * 8;
        }

        if (n > 0)
        {
            for (byte i = 0; i < n; i++)
            {
                backData[i] = readReg(FIFODataReg);
            }
        }
    }

    return 0; // 成功
}

// 请求卡片
byte requestCard(byte *cardType)
{
    byte status;
    byte backBits;
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

// 防碰撞，读取UID
byte anticoll(byte *uid)
{
    byte status;
    byte backBits;
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

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n========================================");
    Serial.println("  RC522 RFID读卡器 (带重试)");
    Serial.println("========================================\n");

    // 初始化I2C（多次尝试）
    pinMode(I2C_SDA, INPUT_PULLUP);
    pinMode(I2C_SCL, INPUT_PULLUP);
    delay(100);

    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(50000); // 降到50kHz
    Wire.setTimeOut(1000);

    Serial.println("初始化I2C... ✅");

    // 复位RC522（延长时间）
    pinMode(RFID_RST, OUTPUT);

    for (int retry = 0; retry < 5; retry++)
    {
        Serial.printf("\n尝试 %d/5...\n", retry + 1);

        digitalWrite(RFID_RST, LOW);
        delay(100); // 延长复位时间
        digitalWrite(RFID_RST, HIGH);
        delay(300); // 延长等待时间

        // 尝试读取版本（多次）
        byte version = 0;
        for (int i = 0; i < 3; i++)
        {
            version = readReg(VersionReg);
            Serial.printf("  读取版本: 0x%02X\n", version);

            if (version != 0x00 && version != 0xFF)
            {
                Serial.printf("\n✅ 成功！版本: 0x%02X\n", version);
                goto init_success;
            }
            delay(100);
        }

        // 重新初始化I2C总线
        Wire.end();
        delay(100);

        pinMode(I2C_SDA, INPUT_PULLUP);
        pinMode(I2C_SCL, INPUT_PULLUP);
        delay(100);

        Wire.begin(I2C_SDA, I2C_SCL);
        Wire.setClock(50000);
        Wire.setTimeOut(1000);
    }

    Serial.println("\n❌ 5次尝试都失败");
    Serial.println("\n请检查：");
    Serial.println("  1. 所有线是否插紧？");
    Serial.println("  2. RC522是否有松动？");
    Serial.println("  3. 电源是否稳定（万用表测3.3V）？");
    while (1)
    {
        delay(1000);
    }

init_success:

    // 软复位
    writeReg(CommandReg, 0x0F);
    delay(50);

    // 配置定时器
    writeReg(TModeReg, 0x80);
    writeReg(TPrescalerReg, 0xA9);
    writeReg(TReloadRegH, 0x03);
    writeReg(TReloadRegL, 0xE8);
    writeReg(TxASKReg, 0x40);
    writeReg(ModeReg, 0x3D);

    // 打开天线
    setBitMask(TxControlReg, 0x03);

    Serial.println("天线已打开 ✅\n");

    Serial.println("========================================");
    Serial.println("请将RFID卡片靠近读卡器（2-3cm内）");
    Serial.println("========================================\n");
}
void loop()
{
    byte cardType[2];
    byte uid[5];

    // 请求卡片
    byte status = requestCard(cardType);

    if (status == 0)
    {
        Serial.println("========================================");
        Serial.println("✅ 检测到RFID卡片！");
        Serial.println("========================================");

        Serial.printf("卡片类型: 0x%02X%02X\n", cardType[0], cardType[1]);

        // 读取UID
        status = anticoll(uid);

        if (status == 0)
        {
            Serial.print("卡片UID: ");
            for (byte i = 0; i < 4; i++)
            {
                if (uid[i] < 0x10)
                    Serial.print("0");
                Serial.print(uid[i], HEX);
                if (i < 3)
                    Serial.print(":");
            }
            Serial.println();
        }
        else
        {
            Serial.println("⚠️  读取UID失败");
        }

        Serial.println("========================================\n");

        delay(2000);
    }

    delay(100);
}