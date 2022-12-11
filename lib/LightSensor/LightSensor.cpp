#include <LightSensor.h>

int isr_flag = 0;

void interruptRoutine()
{
    isr_flag = 1;
};

void initLightSensor(int inter, SparkFun_APDS9960 apds)
{
    pinMode(inter, INPUT);

    attachInterrupt(inter, interruptRoutine, FALLING);
    apds.init();
    apds.enableLightSensor(false);
    apds.setAmbientLightIntEnable(1);
    delay(500);
}

uint16_t getLight(SparkFun_APDS9960 sensor)
{
    uint16_t ambient_light = 0;

    if (isr_flag == 1)
    {
        if (!sensor.readAmbientLight(ambient_light))
        {
            ambient_light = false;
        }

        isr_flag = 0;
        sensor.clearAmbientLightInt();
    }

    return ambient_light;
}