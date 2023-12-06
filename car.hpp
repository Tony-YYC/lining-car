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

    Servo servo;

    int offset = 120; // 电机的零点
    
    const int straight_speed = 125;
    const int turn_speed = 60;
    const int sharp_turn_speed = 30;

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
    void motor_control(int steer)
    {
        servo.write(offset + steer);
        if(abs(steer) > 30)
        {
            A_Motor(HIGH,sharp_turn_speed);
            B_Motor(HIGH,sharp_turn_speed);
        }
        else if(abs(steer) > 15)
        {
            A_Motor(HIGH,turn_speed);
            B_Motor(HIGH,turn_speed);
        }
        else
        {
            A_Motor(HIGH,straight_speed);
            B_Motor(HIGH,straight_speed);
        }
    }

    void run_straight()
    {
        A_Motor(HIGH,straight_speed);
        B_Motor(HIGH,straight_speed);
    }
};

class Car : private Sensor, private Motor
{
  private:
    const float Kp = 10.0;
    const float Ki = 0.2;
    const float Kd = 0.5;

    float error = 0, last_error = 0;
    float integral = 0,pid_output = 0;

    void calc_pid()
    {
        error = Sensor::getStatus() - 3;
        integral += error;
        pid_output = Kp * error + Ki * integral + Kd * (error - last_error);
        last_error = error;
    }

  public:
    Car()
    {
        Motor();
    }

    void run_with_tracing()
    {
        calc_pid();
        Motor::motor_control(static_cast<int>(pid_output));
    }

    void run_without_tracing()
    {
        Motor::run_straight();
        delay(100);
    }
};