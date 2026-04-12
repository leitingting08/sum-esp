#include "heart_sensor.h"

HeartSensor::HeartSensor(uint8_t addr)
{ // ← 修改：接受地址
    i2cAddress = addr;
    beatsPerMinute = 0;
    beatAvg = 0;
    rateSpot = 0;
    lastBeat = 0;
    sensorInitialized = false;

    for (byte i = 0; i < RATE_SIZE; i++)
    {
        rates[i] = 0;
    }
}

bool HeartSensor::begin()
{
    // MAX30105使用固定地址0x57，不需要传参
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        return false;
    }

    particleSensor.setup();
    sensorInitialized = true;
    return true;
}

void HeartSensor::update()
{
    if (!sensorInitialized)
        return;

    long irValue = particleSensor.getIR();

    if (checkForBeat(irValue) == true)
    {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= RATE_SIZE;

            beatAvg = 0;
            for (byte x = 0; x < RATE_SIZE; x++)
            {
                beatAvg += rates[x];
            }
            beatAvg /= RATE_SIZE;
        }
    }
}

int HeartSensor::getHeartRate()
{
    return beatAvg;
}

bool HeartSensor::isFingerDetected()
{
    if (!sensorInitialized)
        return false;
    long irValue = particleSensor.getIR();
    return (irValue > 50000);
}

float HeartSensor::getStressLevel()
{
    if (beatAvg == 0)
        return 0.0;

    if (beatAvg < 60)
    {
        return 0.1;
    }
    else if (beatAvg < 80)
    {
        return 0.3;
    }
    else if (beatAvg < 100)
    {
        return 0.6;
    }
    else
    {
        return 0.9;
    }
}

long HeartSensor::getIRValue()
{
    if (!sensorInitialized)
        return 0;
    return particleSensor.getIR();
}