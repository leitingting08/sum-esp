#include "gsr_sensor.h"

GSRSensor::GSRSensor()
{
    baselineValue = 0;
    isCalibrated = false;
}

void GSRSensor::begin()
{
    pinMode(GSR_PIN, INPUT);
}

int GSRSensor::readRaw()
{
    long sum = 0;
    for (int i = 0; i < 10; i++)
    {
        sum += analogRead(GSR_PIN);
        delay(10);
    }
    return sum / 10;
}

void GSRSensor::calibrate(int duration)
{
    Serial.println("开始校准GSR基线...");
    Serial.printf("请保持放松 %d 秒\n", duration);

    long sum = 0;
    int samples = duration * 2;

    for (int i = 0; i < samples; i++)
    {
        sum += readRaw();
        delay(500);

        if (i % 10 == 0)
        {
            Serial.print(".");
        }
    }

    baselineValue = sum / samples;
    isCalibrated = true;

    Serial.println("");
    Serial.printf("✅ 校准完成！基线值: %d (%.2fV)\n",
                  baselineValue,
                  baselineValue * 3.3 / 4095.0);
}

float GSRSensor::getCognitiveLoad()
{
    if (!isCalibrated)
    {
        return 0.0;
    }

    int currentValue = readRaw();
    float change = (float)(currentValue - baselineValue) / baselineValue;

    // 映射到0-1范围
    // 假设2倍基线 = 满负载
    float load = change / 1.0;

    if (load < 0)
        load = 0;
    if (load > 1)
        load = 1;

    return load;
}

int GSRSensor::getRawValue()
{
    return readRaw();
}

float GSRSensor::getVoltage()
{
    return readRaw() * 3.3 / 4095.0;
}

bool GSRSensor::isReady()
{
    return isCalibrated;
}