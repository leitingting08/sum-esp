#ifndef HEART_SENSOR_H
#define HEART_SENSOR_H

#include <Arduino.h>
#include <MAX30105.h>
#include "heartRate.h"

class HeartSensor
{
private:
    MAX30105 particleSensor;
    uint8_t i2cAddress; // 新增：保存地址

    const byte RATE_SIZE = 4;
    byte rates[4];
    byte rateSpot = 0;
    long lastBeat = 0;
    float beatsPerMinute;
    int beatAvg;

    bool sensorInitialized;

public:
    HeartSensor(uint8_t addr = 0x57); // ← 修改：接受地址参数

    bool begin();
    void update();

    int getHeartRate();
    bool isFingerDetected();
    float getStressLevel();
    long getIRValue();
};

#endif