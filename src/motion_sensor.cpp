#include "motion_sensor.h"
#include "config.h"
#include <Wire.h>

// 通用寄存器定义（适用于MPU6050系列）
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_GYRO_XOUT_H 0x43
#define REG_WHO_AM_I 0x75

MotionSensor::MotionSensor(uint8_t addr)
{
    i2cAddress = addr;
    accelX = accelY = accelZ = 0;
    gyroX = gyroY = gyroZ = 0;
    movementBaseline = 0;
    isCalibrated = false;
}

void MotionSensor::writeReg(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t MotionSensor::readReg(uint8_t reg)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(i2cAddress, (uint8_t)1);
    return Wire.read();
}

void MotionSensor::readRegs(uint8_t reg, uint8_t *buffer, uint8_t len)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(i2cAddress, len);

    for (int i = 0; i < len && Wire.available(); i++)
    {
        buffer[i] = Wire.read();
    }
}

bool MotionSensor::begin()
{
    // 唤醒传感器
    writeReg(REG_PWR_MGMT_1, 0x00);
    delay(100);

    // 验证连接
    uint8_t whoami = readReg(REG_WHO_AM_I);
    if (whoami != 0x68 && whoami != 0x71)
    { // MPU6050或MPU9250
        return false;
    }

    return true;
}

void MotionSensor::update()
{
    uint8_t buffer[14];
    readRegs(REG_ACCEL_XOUT_H, buffer, 14);

    accelX = (buffer[0] << 8) | buffer[1];
    accelY = (buffer[2] << 8) | buffer[3];
    accelZ = (buffer[4] << 8) | buffer[5];

    gyroX = (buffer[8] << 8) | buffer[9];
    gyroY = (buffer[10] << 8) | buffer[11];
    gyroZ = (buffer[12] << 8) | buffer[13];
}

void MotionSensor::getAccel(int16_t &x, int16_t &y, int16_t &z)
{
    x = accelX;
    y = accelY;
    z = accelZ;
}

void MotionSensor::getGyro(int16_t &x, int16_t &y, int16_t &z)
{
    x = gyroX;
    y = gyroY;
    z = gyroZ;
}

void MotionSensor::calibrateFidgeting(int duration)
{
    Serial.println("校准运动基线...");
    Serial.printf("请保持静止 %d 秒\n", duration);

    long sumMovement = 0;
    int samples = duration * 10;

    for (int i = 0; i < samples; i++)
    {
        update();

        long movement = abs(accelX) + abs(accelY) + abs(accelZ) +
                        abs(gyroX) + abs(gyroY) + abs(gyroZ);

        sumMovement += movement;
        delay(100);

        if (i % 50 == 0)
            Serial.print(".");
    }

    movementBaseline = (float)sumMovement / samples;
    isCalibrated = true;

    Serial.println("");
    Serial.printf("✅ 校准完成！基线: %.2f\n", movementBaseline);
}

float MotionSensor::getFidgetingLevel()
{
    if (!isCalibrated)
        return 0.0;

    update();

    long currentMovement = abs(accelX) + abs(accelY) + abs(accelZ) +
                           abs(gyroX) + abs(gyroY) + abs(gyroZ);

    float ratio = (float)currentMovement / movementBaseline;
    float fidgeting = (ratio - 1.0) / 2.0;

    if (fidgeting < 0)
        fidgeting = 0;
    if (fidgeting > 1)
        fidgeting = 1;

    return fidgeting;
}

bool MotionSensor::isReady()
{
    return isCalibrated;
}