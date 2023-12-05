#include "sensor.hpp"
Sensor sensor;
Motor motor;
void setup()
{
    Serial.begin(9600); // 打开串口，设置波特率为9600 bps
}

void loop()
{
    int status = sensor.getStatus();
    Serial.println(status);
    switch(sensor.getStatus())
    {
        case 0:
            motor.stop();
        case 1:
            motor.sharp_left();
        case 2:
            motor.left();
        case 3:
            motor.run();
        case 4:
            motor.right();
        case 5:
            motor.sharp_right();
    }
    delay(100);
}
