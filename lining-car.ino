#include "car.hpp"
Car car;
void setup()
{
    Serial.begin(9600); // 打开串口，设置波特率为9600 bps
    car.init();
}

void loop()
{
    car.run_without_tracing();
}
