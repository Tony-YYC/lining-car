#include <Servo.h>
class Sensor
{
  private:
    const int sensor[5] = {A0, A1, A2, A3, A4};
    const int threshold = 250;
    int sensorReading[5];
    int lineNum = 0;
    int count = 0;
    void read()
    {
        count = 0;
        lineNum = -1;
        for (int i = 0; i < 5; i++)
        {
            sensorReading[i] = analogRead(sensor[i]);
            if (sensorReading[i] < threshold)
                ;
            {
                lineNum = i;
                ++count;
            }
            Serial.print(sensorReading[i]);
            Serial.print('-');
        }
        Serial.println();
    }

  public:
    int getStatus()
    {
        read();
        if (count == 1)
            return lineNum + 1;
        else
            return 0;
    }
};
class Motor
{
  private:
    // A组电机驱动
    const int A_PWM = 6; // 控制速度
    const int A_DIR = 7; // 控制方向
    // B组电机驱动
    const int B_PWM = 5; // 控制速度
    const int B_DIR = 4; // 控制方向

    const int servo_pwm_pin = 9;

    int offset = 120; // 电机的零点

    const int straight_speed = 125;
    const int turn_speed = 60;
    const int sharp_turn_speed = 30;

    const int turn_angle[2] = {15, 15};
    const int sharp_turn_angle[2] = {30, 30};

    Servo servo;

    // A组电机驱动控制函数
    void A_Motor(int dir, int speed)
    {
        digitalWrite(A_DIR, dir);
        analogWrite(A_PWM, speed);
    }

    // B组电机驱动控制函数
    void B_Motor(int dir, int speed)
    {
        digitalWrite(B_DIR, dir);
        analogWrite(B_PWM, speed);
    }

  public:
    Motor()
    {
        pinMode(A_DIR, OUTPUT);
        pinMode(A_PWM, OUTPUT);
        pinMode(B_DIR, OUTPUT);
        pinMode(B_PWM, OUTPUT); // 全部都设置为输出
        servo.attach(servo_pwm_pin);
        offset = servo.read();
    }

    void run()
    {
        A_Motor(HIGH, straight_speed);
        B_Motor(HIGH, straight_speed);
        servo.write(offset);
    }

    void left()
    {
        A_Motor(HIGH, turn_speed);
        B_Motor(HIGH, turn_speed);
        servo.write(offset + turn_angle[0]);
    }

    void right()
    {
        A_Motor(HIGH, turn_speed);
        B_Motor(HIGH, turn_speed);
        servo.write(offset - turn_angle[1]);
    }

    void sharp_left()
    {
        A_Motor(HIGH, sharp_turn_speed);
        B_Motor(HIGH, sharp_turn_speed);
        servo.write(offset + sharp_turn_angle[0]);
    }

    void sharp_right()
    {
        A_Motor(HIGH, sharp_turn_speed);
        B_Motor(HIGH, sharp_turn_speed);
        servo.write(offset - sharp_turn_angle[1]);
    }

    void stop()
    {
        A_Motor(HIGH, 0);
        B_Motor(HIGH, 0);
        servo.write(offset);
    }
};

class Car : private Sensor, private Motor
{
  public:
    Car()
    {
        Motor();
    }

    void run_with_tracing()
    {
        int status = Sensor::getStatus();
        Serial.println(status);
        switch (Sensor::getStatus())
        {
        case 0:
            Motor::stop();
        case 1:
            Motor::sharp_left();
        case 2:
            Motor::left();
        case 3:
            Motor::run();
        case 4:
            Motor::right();
        case 5:
            Motor::sharp_right();
        }
    }

    void run_without_tracing()
    {
        Motor::run();
    }
}
;