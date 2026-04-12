#ifndef GSR_SENSOR_H
#define GSR_SENSOR_H

#include <Arduino.h>
#include "config.h"

class GSRSensor
{
private:
    int baselineValue;
    bool isCalibrated;

    int readRaw();

public:
    GSRSensor();

    void begin();
    void calibrate(int duration = 30);
    float getCognitiveLoad(); // 0-1
    int getRawValue();
    float getVoltage();
    bool isReady();
};

#endif