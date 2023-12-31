#include <Servo.h>
class Sensor
{
  private:
    const int sensor[5] = {A0, A1, A2, A3, A4};
    const int threshold = 600;
    int sensorReading[5];
    int lineNum = 0;
    int lineNum_2 = 0;
    int count = 0;
    void read()
    {
        count = 0;
        for (int i = 0; i < 5; i++)
        {
            sensorReading[i] = analogRead(sensor[i]);
            if (sensorReading[i] < threshold)
            {
                if (count == 1)
                {
                    ++count;
                    lineNum_2 = i;
                }
                else if (count == 0)
                {
                    ++count;
                    lineNum = i;
                }
                else
                {
                    ++count;
                }
            }
            Serial.print(sensorReading[i]);
            Serial.print('-');
        }
        Serial.println();
    }

  public:
    float getStatus()
    {
        read();
        if (count == 1)
            return lineNum + 1;
        else if (count == 2)
            return float(lineNum + lineNum_2) / 2 + 1;
        else if (count == 5)
            return -1;
        else
            return 0;
    }
};
class Motor
{
  private:
    // A是右电机，B是左电机
    //  A组电机驱动
    const int A_PWM = 6; // 控制速度
    const int A_DIR = 7; // 控制方向
    const int A_DIRECTION = LOW;

    // B组电机驱动
    const int B_PWM = 5; // 控制速度
    const int B_DIR = 4; // 控制方向
    const int B_DIRECTION = LOW;

    const int servo_pwm_pin = 9;
    const int SERVO_LEFT_MAX = 32;
    const int SERVO_RIGHT_MAX = -60;

    Servo servo;

    int offset = 120; // 电机的零点

    const int straight_speed = 220;
    const int light_turn_speed = 185;
    const int turn_speed = 145;
    const int sharp_turn_speed = 105;

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
    void A_Motor_DIR(int dir, int speed)
    {
        if (speed >= 0)
        {
            digitalWrite(A_DIR, dir);
            analogWrite(A_PWM, speed);
        }
        else
        {
            digitalWrite(A_DIR, !dir);
            analogWrite(A_PWM, -speed);
        }
    }
    void B_Motor_DIR(int dir, int speed)
    {
        if (speed >= 0)
        {
            digitalWrite(B_DIR, dir);
            analogWrite(B_PWM, speed);
        }
        else
        {
            digitalWrite(B_DIR, !dir);
            analogWrite(B_PWM, -speed);
        }
    }

  public:
    void init()
    {
        pinMode(A_DIR, OUTPUT);
        pinMode(A_PWM, OUTPUT);
        pinMode(B_DIR, OUTPUT);
        pinMode(B_PWM, OUTPUT); // 全部都设置为输出
        servo.attach(servo_pwm_pin);
    }
    bool motor_control(int steer)
    {
        bool touch_limit = false;
        if (steer < 0)
            steer *= 1.1;
        if (steer > SERVO_LEFT_MAX)
            steer = SERVO_LEFT_MAX; // 舵机输出限幅
            touch_limit = true;
        if (steer < SERVO_RIGHT_MAX)
            steer = SERVO_RIGHT_MAX;
            touch_limit = true;
        if (abs(steer) > 30)
        {
            A_Motor_DIR(A_DIRECTION, sharp_turn_speed + 1.2 * steer); // 后轮差速辅助转向
            B_Motor_DIR(B_DIRECTION, sharp_turn_speed - 1.2 * steer);
        }
        else if (abs(steer) > 20)
        {
            A_Motor_DIR(A_DIRECTION, turn_speed + 0.5 * steer);
            B_Motor_DIR(B_DIRECTION, turn_speed - 0.5 * steer);
        }
        else if (abs(steer) > 10)
        {
            A_Motor(A_DIRECTION, light_turn_speed + 0.2 * steer);
            B_Motor(B_DIRECTION, light_turn_speed - 0.2 * steer);
        }
        else
        {
            A_Motor(A_DIRECTION, straight_speed);
            B_Motor(B_DIRECTION, straight_speed);
        }
        Serial.print("steer:");
        Serial.println(steer);
        servo.write(offset + steer);
        return touch_limit;
    }

    void run_straight()
    {
        servo.write(offset);
        A_Motor(A_DIRECTION, straight_speed);
        B_Motor(B_DIRECTION, straight_speed);
    }
};

class Car : protected Sensor, protected Motor
{
  private:
    const float Kp = 18.0;
    const float Ki = 0.26 / 2;
    const float Kd = 46.0 * 2;

    float error = 0, last_error = 0;
    float integral = 0, pid_output = 0;
    bool anti_saturation = false;

    const float INTEGRAL_MAX = 120.0;

    void calc_pid()
    {
        error = Sensor::getStatus();
        Serial.print("error:");
        Serial.print(error);
        if (error > 0)
        {
            error -= 3;
            integral += error;
            if (integral > INTEGRAL_MAX)
                integral = INTEGRAL_MAX; // 积分限幅
            if (integral < -INTEGRAL_MAX)
                integral = -INTEGRAL_MAX;
            //if(error * integral < 0) integral = 0; //积分与当前值反向，则积分值清零
            pid_output = Kp * error + Ki * integral + Kd * (error - last_error);
            last_error = error;

            Serial.print("--pid_output:");
            Serial.print(pid_output);
            Serial.println();
        }
        else if (error == 0)
        {
            error = last_error;
            integral += error;
            if (integral > INTEGRAL_MAX)
                integral = INTEGRAL_MAX; // 积分限幅
            if (integral < -INTEGRAL_MAX)
                integral = -INTEGRAL_MAX;
            float delta;
            if (error == 2 && integral > 0)
                delta == 1;
            else if (error == -2 && integral < 0)
                delta == -1;
            pid_output = Kp * error + Ki * integral + Kd * delta;
        }
        else
        {
            integral = 0;
            last_error = 0;
            pid_output = 0;
            error = 0;
            Motor::motor_control(0);
            delay(20);
        }
    }

  public:
    void start()
    {
        Motor::init();
    }

    void run_with_tracing()
    {
        calc_pid();
        anti_saturation = Motor::motor_control(static_cast<int>(pid_output));
        delay(5);
    }

    void run_without_tracing()
    {
        Motor::run_straight();
        delay(100);
    }
};