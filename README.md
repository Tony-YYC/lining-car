# lining-car
lining car code for SI1210
## branch introduction
- direct_control branch use some switches (if-else) to control the steering of the car
- pid branch use pid to calculate the steering of the car
## What is pid?
**P means Portion, I means integral, D means differentiation**\
discrete form of pid:
$$
u(k) = K_pe(t) + K_i \sum_{n=0}^k e(n) + K_d[e(k)-e(k-1)]
$$
## Some tips for tuning pid
```
参数整定找最佳，从小到大顺序查 
先是比例后积分，最后再把微分加 
曲线振荡很频繁，比例度盘要放大(Kp减小) 
曲线漂浮绕大湾，比例度盘往小扳(Kp增大)
曲线偏离回复慢，积分时间往下降(Ki增大)
曲线波动周期长，积分时间再加长(Ki减小)
曲线振荡频率快，先把微分降下来(Kd增大) 
动差大来波动慢。微分时间应加长(Kd减小) 
理想曲线两个波，前高后低4比1 
一看二调多分析，调节质量不会低
```

## 程序结构

```mermaid
classDiagram
    Sensor --|> Car : 继承
    Motor --|> Car : 继承

    class Sensor {
        -int sensorReading[5] //5个灰度传感器数据
        -int lineNum //线条在传感器下的编号
        -int lineNum_2 //第2根线条在传感器下的编号
        -int count //线条计数
        -const int threshod //灰度阈值
        -void read() //读取原始灰度数据
        +float getStatus() //获取传感器读取到线的位置
    }

    class Motor {
        -const int servo_pwm_pin
        -const int SERVO_LEFT_MAX //舵机左转最大限位
        -const int SERVO_RIGHT_MAX //舵机右转最大限位
        -const int A_DIRECTION
        -const int B_DIRECTION
        -const int straight_speed //直行速度
        -const int turn_speed //转小弯速度
        -const int sharp_turn_speed //转大弯速度
        -Servo servo
        -int offset
        -void A_Motor(int dir, int speed)
        -void B_Motor(int dir, int speed)
        +void init() //初始化
        +void motor_control(int steer) //输入角度，控制舵机和电机
        +void run_straight() //只走直线
    }

    class Car {
        -float error //当前差值
        -float last_error //上次插值
        -float integral //积分器
        -float pid_output //pid输出
        -const float Kp  //pid比例项系数
        -const float Ki  //pid积分项系数
        -const float Kd  //pid微分项系数
        -void calc_pid() //计算pid的函数
        +void start() //初始化函数
        +void run_with_tracing() //带巡线功能跑
        +void run_without_tracing() //不带巡线功能跑
    }

```

```mermaid
graph TD;
    O --> I[电机转动控制]
    N --> S[舵机转动控制]
    R -->|返回电机是否达到最大限位，适时暂停积分器| D

    subgraph "PID 控制"
    A[calc_pid] -->|获取状态| B[Sensor::getStatus]
    B --> C[计算巡线的误差]
    C --> D[更新积分器]
    C --> E[计算微分]
    D --> F[全部相加，得到PID输出]
    E --> F
    C --> Q[计算比例项]
    Q --> F
    F --> G[更新上一次误差]
    end

    subgraph "传感器读取"
    J[read] --> K[判断黑线位置]
    K -->|返回状态| B
    end

    subgraph "电机控制"
    H[Motor::motor_control] --> L
    F --> L
    L[接收PID输出] --> R[转向角度检测，确保不超过最大限位]
    R --> M[计算舵机和电机控制参数]
    M -->|调整舵机| N[调整舵机方向]
    M -->|控制电机| O[控制电机速度和方向]
    end

```