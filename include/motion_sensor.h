#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

// 可以是MPU6050、MPU9250等任何IMU传感器
class MotionSensor
{
private:
    uint8_t i2cAddress;

    void writeReg(uint8_t reg, uint8_t value);
    uint8_t readReg(uint8_t reg);
    void readRegs(uint8_t reg, uint8_t *buffer, uint8_t len);

    int16_t accelX, accelY, accelZ;
    int16_t gyroX, gyroY, gyroZ;

    float movementBaseline;
    bool isCalibrated;

public:
    MotionSensor(uint8_t addr = 0x68); // 默认MPU6050地址

    bool begin();
    void update();

    // 获取原始数据
    void getAccel(int16_t &x, int16_t &y, int16_t &z);
    void getGyro(int16_t &x, int16_t &y, int16_t &z);

    // Fidgeting检测（坐立不安指标）
    void calibrateFidgeting(int duration = 30);
    float getFidgetingLevel(); // 0-1，越高越坐立不安
    bool isReady();
};

#endif