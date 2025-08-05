//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-- Otto DIY Serial Control - Arduino Nano Version
//-- 基于xiaozhi-esp32 otto-robot功能的Arduino Nano串口控制实现
//-- 支持与ESP32通过串口通信控制Otto机器人的所有动作
//-- 
//-- 硬件要求:
//-- * Arduino Nano (ATmega328P)
//-- * 6个微型舵机 (SG90或类似)
//-- * 蜂鸣器 (可选)
//-- * 独立5V电源 (舵机用，推荐3-5A)
//-- 
//-- 引脚连接 (优化PWM分配):
//-- * 左腿舵机  -> D3  (PWM支持)
//-- * 右腿舵机  -> D5  (PWM支持)
//-- * 左脚舵机  -> D6  (PWM支持)
//-- * 右脚舵机  -> D9  (PWM支持)
//-- * 左手舵机  -> D10 (PWM支持)
//-- * 右手舵机  -> D11 (PWM支持)
//-- * 蜂鸣器    -> D13 (内置LED可共用)
//-- * 状态LED   -> D13 (内置LED)
//-- * 串口通信  -> D0(RX), D1(TX) 硬件串口
//-- 
//-- 电源连接:
//-- * Arduino: USB或VIN (7-12V)
//-- * 舵机: 独立5V电源，与Arduino共地
//-- 
//-- 串口协议格式: COMMAND param1 param2 param3...\n
//-- 波特率: 115200
//-- 注意: 使用空格分隔命令和参数，不要使用冒号和逗号
//-- 
//-- 支持的命令:
//-- INIT - 初始化机器人
//-- HOME hands_down - 回到初始位置 (hands_down: 0=保持手部位置, 1=放下手臂)
//-- WALK steps speed direction amount - 行走 (direction: 1=前进, -1=后退; amount: 手臂摆动幅度)
//-- MOVE_FORWARD steps speed - 简化前进命令（默认带手臂摆动）
//-- MOVE_BACKWARD steps speed - 简化后退命令（默认带手臂摆动）
//-- TURN steps speed direction amount - 转向 (direction: 1=左转, -1=右转)
//-- TURN_LEFT steps speed - 简化左转命令
//-- TURN_RIGHT steps speed - 简化右转命令
//-- JUMP steps speed - 跳跃
//-- SWING steps speed height - 摇摆
//-- MOONWALK steps speed height direction - 太空步
//-- BEND steps speed direction - 弯曲 (direction: 1=左, -1=右)
//-- SHAKE_LEG steps speed direction - 摇腿
//-- UPDOWN steps speed height - 上下运动
//-- TIPTOE_SWING steps speed height - 踮脚摇摆
//-- JITTER steps speed height - 抖动
//-- ASCENDING_TURN steps speed height - 上升转向
//-- CRUSAITO steps speed height direction - 十字步
//-- FLAPPING steps speed height direction - 拍打动作
//-- HANDS_UP speed direction - 举手 (仅支持6舵机版本)
//-- HANDS_DOWN speed direction - 放手
//-- HAND_WAVE speed direction - 挥手 (direction: 1=左手, -1=右手, 0=双手)
//-- SET_TRIMS yl yr rl rr lh rh - 设置舵机微调
//-- ENABLE_LIMIT speed_limit - 启用舵机速度限制 (degree/sec)
//-- DISABLE_LIMIT - 禁用舵机速度限制
//-- GET_STATUS - 获取机器人状态
//-- SERVO_MOVE servo position - 单独控制舵机 (servo: 0-5, position: 0-180)
//-- STOP - 停止当前动作
//-- 
//-- 返回格式:
//-- OK:command - 命令执行成功
//-- ERROR:message - 命令执行失败
//-- STATUS:info - 状态信息
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <SerialCommand.h>
#include <Otto.h>
#include <Servo.h>

// 引脚定义 - 兼容Arduino Nano (优化PWM分配)
#define LEFT_LEG_PIN    3   // 左腿舵机 (PWM)
#define RIGHT_LEG_PIN   5   // 右腿舵机 (PWM)  
#define LEFT_FOOT_PIN   6   // 左脚舵机 (PWM)
#define RIGHT_FOOT_PIN  9   // 右脚舵机 (PWM)
#define LEFT_HAND_PIN   10  // 左手舵机 (PWM)
#define RIGHT_HAND_PIN  11  // 右手舵机 (PWM)
#define BUZZER_PIN      13  // 蜂鸣器
#define STATUS_LED_PIN  LED_BUILTIN // 状态LED (D13)

// 舵机索引定义
#define LEFT_LEG    0
#define RIGHT_LEG   1  
#define LEFT_FOOT   2
#define RIGHT_FOOT  3
#define LEFT_HAND   4
#define RIGHT_HAND  5
#define SERVO_COUNT 6

// 默认手部位置
#define HAND_HOME_POSITION 45

// 方向常量
#define FORWARD  1
#define BACKWARD -1
#define LEFT     1
#define RIGHT    -1

SerialCommand SCmd;
Otto ottoRobot;

// 全局状态变量
bool is_executing_command = false;
unsigned long last_command_time = 0;
const unsigned long COMMAND_TIMEOUT = 10000; // 10秒超时

// 扩展的舵机控制类，支持6个舵机
class ExtendedOtto {
private:
    int servo_trims[2]; // 只保存手部舵机微调
    bool has_hands;
    bool is_resting;
    Servo hand_servos[2]; // 手臂舵机对象
    
public:
    ExtendedOtto() {
        // 初始化微调值
        servo_trims[0] = 0; // 左手
        servo_trims[1] = 0; // 右手
        
        has_hands = true; // Arduino版本支持手部舵机
        is_resting = false;
    }
    
    void init() {
        // 使用Otto库初始化基础4个舵机
        ottoRobot.init(LEFT_LEG_PIN, RIGHT_LEG_PIN, LEFT_FOOT_PIN, RIGHT_FOOT_PIN, true, BUZZER_PIN);
        
        // 初始化手部舵机
        if(has_hands) {
            hand_servos[0].attach(LEFT_HAND_PIN);   // 左手舵机
            hand_servos[1].attach(RIGHT_HAND_PIN);  // 右手舵机
            
            // 设置手部舵机到默认位置
            hand_servos[0].write(HAND_HOME_POSITION);
            hand_servos[1].write(180 - HAND_HOME_POSITION);
            delay(500);
        }
        
        is_resting = false;
        Serial.println(F("OK:INIT"));
    }
    
    void home(bool hands_down = true) {
        ottoRobot.home();
        
        if(has_hands && hands_down) {
            // 手部复位到默认位置
            moveServoToPosition(LEFT_HAND, HAND_HOME_POSITION);
            moveServoToPosition(RIGHT_HAND, 180 - HAND_HOME_POSITION);
            delay(500);
        }
        
        is_resting = true;
        Serial.println(F("OK:HOME"));
    }
    
    void setTrims(int yl, int yr, int rl, int rr, int lh = 0, int rh = 0) {
        // 设置基础4个舵机的微调
        ottoRobot.setTrims(yl, yr, rl, rr);
        
        // 保存手部舵机微调
        servo_trims[0] = lh; // 左手
        servo_trims[1] = rh; // 右手
        
        Serial.println(F("OK:SET_TRIMS"));
    }
    
    void enableServoLimit(int speed_limit = 240) {
        ottoRobot.enableServoLimit(speed_limit);
        Serial.println(F("OK:ENABLE_LIMIT"));
    }
    
    void disableServoLimit() {
        ottoRobot.disableServoLimit();
        Serial.println(F("OK:DISABLE_LIMIT"));
    }
    
    // 基础动作函数 - 直接使用Otto库
    void walk(float steps, int speed, int direction, int amount = 0) {
        if(is_resting) is_resting = false;
        
        // 防止命令执行时间过长
        is_executing_command = true;
        last_command_time = millis();
        
        // 如果有手臂摆动需求且有手部舵机
        if(amount > 0 && has_hands) {
            // 实现简化的手臂摆动：在走路过程中让手臂轻微摆动
            walkWithArmSwing(steps, speed, direction, amount);
        } else {
            ottoRobot.walk(steps, speed, direction);
        }
        
        is_executing_command = false;
        Serial.println(F("OK:WALK"));
    }
    
    void turn(float steps, int speed, int direction, int amount = 0) {
        if(is_resting) is_resting = false;
        
        // 防止命令执行时间过长
        is_executing_command = true;
        last_command_time = millis();
        
        // 如果有手臂摆动需求且有手部舵机
        if(amount > 0 && has_hands) {
            // 实现简化的转向手臂摆动
            turnWithArmSwing(steps, speed, direction, amount);
        } else {
            ottoRobot.turn(steps, speed, direction);
        }
        
        is_executing_command = false;
        Serial.println(F("OK:TURN"));
    }
    
    void jump(float steps, int speed) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.jump(steps, speed);
        
        is_executing_command = false;
        Serial.println(F("OK:JUMP"));
    }
    
    void swing(float steps, int speed, int height) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.swing(steps, speed, height);
        
        is_executing_command = false;
        Serial.println(F("OK:SWING"));
    }
    
    void moonwalker(float steps, int speed, int height, int direction) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.moonwalker(steps, speed, height, direction);
        
        is_executing_command = false;
        Serial.println(F("OK:MOONWALK"));
    }
    
    void bend(float steps, int speed, int direction) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.bend(steps, speed, direction);
        
        is_executing_command = false;
        Serial.println(F("OK:BEND"));
    }
    
    void shakeLeg(float steps, int speed, int direction) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.shakeLeg(steps, speed, direction);
        
        is_executing_command = false;
        Serial.println(F("OK:SHAKE_LEG"));
    }
    
    void upDown(float steps, int speed, int height) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.updown(steps, speed, height);
        
        is_executing_command = false;
        Serial.println(F("OK:UPDOWN"));
    }
    
    void tiptoeSwing(float steps, int speed, int height) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.tiptoeSwing(steps, speed, height);
        
        is_executing_command = false;
        Serial.println(F("OK:TIPTOE_SWING"));
    }
    
    void jitter(float steps, int speed, int height) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.jitter(steps, speed, height);
        
        is_executing_command = false;
        Serial.println(F("OK:JITTER"));
    }
    
    void ascendingTurn(float steps, int speed, int height) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.ascendingTurn(steps, speed, height);
        
        is_executing_command = false;
        Serial.println(F("OK:ASCENDING_TURN"));
    }
    
    void crusaito(float steps, int speed, int height, int direction) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.crusaito(steps, speed, height, direction);
        
        is_executing_command = false;
        Serial.println(F("OK:CRUSAITO"));
    }
    
    void flapping(float steps, int speed, int height, int direction) {
        if(is_resting) is_resting = false;
        
        is_executing_command = true;
        last_command_time = millis();
        
        ottoRobot.flapping(steps, speed, height, direction);
        
        is_executing_command = false;
        Serial.println(F("OK:FLAPPING"));
    }
    
    // 手部动作 - 扩展功能
    void handsUp(int speed = 1000, int direction = 0) {
        if(!has_hands) {
            Serial.println(F("ERROR:No hand servos configured"));
            return;
        }
        
        if(is_resting) is_resting = false;
        
        // direction: 0=双手, 1=左手, -1=右手
        // 修正手臂角度：左手170度举起，右手10度举起（镜像关系）
        if(direction == 0 || direction == 1) {
            moveServoToPosition(LEFT_HAND, 170, speed);
        }
        if(direction == 0 || direction == -1) {
            moveServoToPosition(RIGHT_HAND, 10, speed);
        }
        
        Serial.println(F("OK:HANDS_UP"));
    }
    
    void handsDown(int speed = 1000, int direction = 0) {
        if(!has_hands) {
            Serial.println(F("ERROR:No hand servos configured"));
            return;
        }
        
        if(is_resting) is_resting = false;
        
        if(direction == 0 || direction == 1) {
            moveServoToPosition(LEFT_HAND, HAND_HOME_POSITION, speed);
        }
        if(direction == 0 || direction == -1) {
            moveServoToPosition(RIGHT_HAND, 180 - HAND_HOME_POSITION, speed);
        }
        
        Serial.println(F("OK:HANDS_DOWN"));
    }
    
    void handWave(int speed = 1000, int direction = LEFT) {
        if(!has_hands) {
            Serial.println(F("ERROR:No hand servos configured"));
            return;
        }
        
        if(is_resting) is_resting = false;
        
        // 首先将要挥手的手臂举起
        if(direction == LEFT || direction == 0) {
            moveServoToPosition(LEFT_HAND, 170, speed/3);
        }
        if(direction == RIGHT || direction == 0) {
            moveServoToPosition(RIGHT_HAND, 10, speed/3);
        }
        delay(speed/3);
        
        // 挥手动作 - 左右摆动
        for(int i = 0; i < 3; i++) {
            if(direction == LEFT || direction == 0) {
                moveServoToPosition(LEFT_HAND, 140, speed/6);  // 向下30度
                delay(speed/6);
                moveServoToPosition(LEFT_HAND, 180, speed/6);  // 向上到最大角度
                delay(speed/6);
            }
            if(direction == RIGHT || direction == 0) {
                moveServoToPosition(RIGHT_HAND, 40, speed/6);   // 向上30度
                delay(speed/6);
                moveServoToPosition(RIGHT_HAND, 0, speed/6);    // 向下到最小角度
                delay(speed/6);
            }
        }
        
        // 回到默认位置
        handsDown(speed/2, direction);
        
        Serial.println(F("OK:HAND_WAVE"));
    }
    
    void moveServo(int servo_num, int position) {
        if(servo_num < 0 || servo_num >= SERVO_COUNT) {
            Serial.println(F("ERROR:Invalid servo number"));
            return;
        }
        
        if(position < 0 || position > 180) {
            Serial.println(F("ERROR:Invalid position"));
            return;
        }
        
        // 对于基础4个舵机，使用Otto库的功能
        if(servo_num < 4) {
            ottoRobot._moveSingle(position, servo_num);
        } else {
            // 手部舵机直接控制
            moveServoToPosition(servo_num, position);
        }
        
        Serial.println(F("OK:SERVO_MOVE"));
    }
    
    void getStatus() {
        Serial.print(F("STATUS:resting="));
        Serial.print(is_resting ? F("true") : F("false"));
        Serial.print(F(",hands="));
        Serial.print(has_hands ? F("true") : F("false"));
        Serial.print(F(",exec="));
        Serial.print(is_executing_command ? F("true") : F("false"));
        Serial.println();
    }
    
    void stopMovement() {
        // 停止所有运动
        is_executing_command = false;
        home(false); // 不强制放下手臂
        Serial.println(F("OK:STOP"));
    }
    
    void detachServos() {
        // 分离手臂舵机
        if(has_hands) {
            if(hand_servos[0].attached()) hand_servos[0].detach();
            if(hand_servos[1].attached()) hand_servos[1].detach();
        }
        
        // 分离Otto库管理的舵机
        ottoRobot.detachServos();
    }
    
private:
    void moveServoToPosition(int servo_index, int position, int duration = 200) {
        // 应用微调 - 只处理手部舵机
        if(servo_index == LEFT_HAND) {
            position += servo_trims[0];
        } else if(servo_index == RIGHT_HAND) {
            position += servo_trims[1];
        }
        position = constrain(position, 0, 180);
        
        // 对于手臂舵机，使用Servo库控制
        if(servo_index == LEFT_HAND && hand_servos[0].attached()) {
            hand_servos[0].write(position);
        } else if(servo_index == RIGHT_HAND && hand_servos[1].attached()) {
            hand_servos[1].write(position);
        }
        
        // 使用非阻塞延时以实现平滑运动
        if(duration > 0) {
            duration = constrain(duration, 20, 1000); // 限制延时范围
            unsigned long start_time = millis();
            while(millis() - start_time < duration) {
                if(Serial.available()) {
                    // 不处理串口，避免递归
                    break;
                }
                yield();
            }
        }
    }
    
    // 带手臂摆动的走路功能
    void walkWithArmSwing(float steps, int speed, int direction, int amount) {
        // 限制最大步数防止卡死
        steps = constrain(steps, 1, 10);
        
        // 计算每步的持续时间
        int step_duration = constrain(speed / 4, 50, 500);  // 限制延时范围
        
        // 计算手臂摆动的范围
        int swing_range = constrain(amount, 10, 60);  // 限制摆动幅度
        int left_center = HAND_HOME_POSITION;
        int right_center = 180 - HAND_HOME_POSITION;
        
        for(int step = 0; step < steps; step++) {
            // 检查是否需要中断执行
            if(millis() - last_command_time > COMMAND_TIMEOUT) {
                Serial.println(F("ERROR:Command timeout"));
                break;
            }
            
            // 模拟走路的手臂摆动：左手与右腿同步，右手与左腿同步
            if(step % 2 == 0) {  // 偶数步
                // 左手前摆，右手后摆
                moveServoToPosition(LEFT_HAND, left_center - swing_range/2, step_duration/4);
                moveServoToPosition(RIGHT_HAND, right_center + swing_range/2, step_duration/4);
            } else {  // 奇数步
                // 左手后摆，右手前摆
                moveServoToPosition(LEFT_HAND, left_center + swing_range/2, step_duration/4);
                moveServoToPosition(RIGHT_HAND, right_center - swing_range/2, step_duration/4);
            }
            
            // 执行一步基础走路动作，使用较小的延时
            ottoRobot.walk(1, speed, direction);
            
            // 非阻塞延时，允许串口处理
            unsigned long start_time = millis();
            while(millis() - start_time < 50) {
                // 处理可能的串口数据
                if(Serial.available()) {
                    SCmd.readSerial();
                }
                yield(); // ESP32兼容性
            }
        }
        
        // 恢复手臂到默认位置
        moveServoToPosition(LEFT_HAND, left_center, 200);
        moveServoToPosition(RIGHT_HAND, right_center, 200);
    }
    
    // 带手臂摆动的转向功能
    void turnWithArmSwing(float steps, int speed, int direction, int amount) {
        // 限制最大步数防止卡死
        steps = constrain(steps, 1, 5);
        
        // 计算手臂摆动的范围
        int swing_range = constrain(amount, 10, 60);
        int left_center = HAND_HOME_POSITION;
        int right_center = 180 - HAND_HOME_POSITION;
        
        for(int step = 0; step < steps; step++) {
            // 检查是否需要中断执行
            if(millis() - last_command_time > COMMAND_TIMEOUT) {
                Serial.println(F("ERROR:Command timeout"));
                break;
            }
            
            // 转向时的手臂摆动：增强转向效果
            if(direction == LEFT) {  // 左转
                // 左手向后，右手向前，增强左转效果
                moveServoToPosition(LEFT_HAND, left_center + swing_range/2, speed/8);
                moveServoToPosition(RIGHT_HAND, right_center - swing_range/2, speed/8);
            } else {  // 右转
                // 左手向前，右手向后，增强右转效果
                moveServoToPosition(LEFT_HAND, left_center - swing_range/2, speed/8);
                moveServoToPosition(RIGHT_HAND, right_center + swing_range/2, speed/8);
            }
            
            // 执行一步转向动作
            ottoRobot.turn(1, speed, direction);
            
            // 非阻塞延时
            unsigned long start_time = millis();
            while(millis() - start_time < 50) {
                if(Serial.available()) {
                    SCmd.readSerial();
                }
                yield();
            }
        }
        
        // 恢复手臂到默认位置
        moveServoToPosition(LEFT_HAND, left_center, 200);
        moveServoToPosition(RIGHT_HAND, right_center, 200);
    }
};

ExtendedOtto extendedOtto;

// 串口命令处理函数
void handleInit() {
    extendedOtto.init();
}

void handleHome() {
    char *arg = SCmd.next();
    bool hands_down = (arg != NULL) ? (atoi(arg) == 1) : true;
    extendedOtto.home(hands_down);
}

void handleWalk() {
    float steps = 1;
    int speed = 1000;
    int direction = FORWARD;
    int amount = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) amount = atoi(arg);
    
    extendedOtto.walk(steps, speed, direction, amount);
}

// 新增：简化的前进命令
void handleMoveForward() {
    float steps = 3;
    int speed = 1200;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    extendedOtto.walk(steps, speed, FORWARD, 30); // 默认带手臂摆动
}

// 新增：简化的后退命令
void handleMoveBackward() {
    float steps = 3;
    int speed = 1200;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    extendedOtto.walk(steps, speed, BACKWARD, 30); // 默认带手臂摆动
}

void handleTurn() {
    float steps = 1;
    int speed = 2000;
    int direction = LEFT;
    int amount = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) amount = atoi(arg);
    
    extendedOtto.turn(steps, speed, direction, amount);
}

// 新增：简化的左转命令
void handleTurnLeft() {
    float steps = 2;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    extendedOtto.turn(steps, speed, LEFT, 0);
}

// 新增：简化的右转命令
void handleTurnRight() {
    float steps = 2;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    extendedOtto.turn(steps, speed, RIGHT, 0);
}

void handleJump() {
    float steps = 1;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    extendedOtto.jump(steps, speed);
}

void handleSwing() {
    float steps = 1;
    int speed = 1000;
    int height = 20;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    extendedOtto.swing(steps, speed, height);
}

void handleMoonwalk() {
    float steps = 1;
    int speed = 900;
    int height = 20;
    int direction = LEFT;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.moonwalker(steps, speed, height, direction);
}

void handleBend() {
    float steps = 1;
    int speed = 1400;
    int direction = LEFT;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.bend(steps, speed, direction);
}

void handleShakeLeg() {
    float steps = 1;
    int speed = 2000;
    int direction = RIGHT;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.shakeLeg(steps, speed, direction);
}

void handleUpDown() {
    float steps = 1;
    int speed = 1000;
    int height = 20;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    extendedOtto.upDown(steps, speed, height);
}

void handleTiptoeSwing() {
    float steps = 1;
    int speed = 900;
    int height = 20;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    extendedOtto.tiptoeSwing(steps, speed, height);
}

void handleJitter() {
    float steps = 1;
    int speed = 500;
    int height = 20;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    extendedOtto.jitter(steps, speed, height);
}

void handleAscendingTurn() {
    float steps = 1;
    int speed = 900;
    int height = 20;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    extendedOtto.ascendingTurn(steps, speed, height);
}

void handleCrusaito() {
    float steps = 1;
    int speed = 900;
    int height = 20;
    int direction = FORWARD;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.crusaito(steps, speed, height, direction);
}

void handleFlapping() {
    float steps = 1;
    int speed = 1000;
    int height = 20;
    int direction = FORWARD;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) height = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.flapping(steps, speed, height, direction);
}

void handleHandsUp() {
    int speed = 1000;
    int direction = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.handsUp(speed, direction);
}

void handleHandsDown() {
    int speed = 1000;
    int direction = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.handsDown(speed, direction);
}

void handleHandWave() {
    int speed = 1000;
    int direction = LEFT;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    extendedOtto.handWave(speed, direction);
}

void handleSetTrims() {
    int yl = 0, yr = 0, rl = 0, rr = 0, lh = 0, rh = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) yl = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) yr = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) rl = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) rr = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) lh = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) rh = atoi(arg);
    
    extendedOtto.setTrims(yl, yr, rl, rr, lh, rh);
}

void handleEnableLimit() {
    int speed_limit = 240;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed_limit = atoi(arg);
    
    extendedOtto.enableServoLimit(speed_limit);
}

void handleDisableLimit() {
    extendedOtto.disableServoLimit();
}

void handleGetStatus() {
    extendedOtto.getStatus();
}

void handleServoMove() {
    int servo = -1;
    int position = 90;
    
    char *arg = SCmd.next();
    if(arg != NULL) servo = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) position = atoi(arg);
    
    if(servo >= 0) {
        extendedOtto.moveServo(servo, position);
    } else {
        Serial.println(F("ERROR:Missing servo number"));
    }
}

void handleStop() {
    extendedOtto.stopMovement();
}

void handleDefault() {
    Serial.println(F("ERROR:Unknown command"));
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    
    // 初始化状态LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // 注册串口命令
    SCmd.addCommand("INIT", handleInit);
    SCmd.addCommand("HOME", handleHome);
    SCmd.addCommand("WALK", handleWalk);
    SCmd.addCommand("MOVE_FORWARD", handleMoveForward);   // 新增
    SCmd.addCommand("MOVE_BACKWARD", handleMoveBackward); // 新增
    SCmd.addCommand("TURN", handleTurn);
    SCmd.addCommand("TURN_LEFT", handleTurnLeft);         // 新增
    SCmd.addCommand("TURN_RIGHT", handleTurnRight);       // 新增
    SCmd.addCommand("JUMP", handleJump);
    SCmd.addCommand("SWING", handleSwing);
    SCmd.addCommand("MOONWALK", handleMoonwalk);
    SCmd.addCommand("HANDS_UP", handleHandsUp);
    SCmd.addCommand("HANDS_DOWN", handleHandsDown);
    SCmd.addCommand("HAND_WAVE", handleHandWave);
    SCmd.addCommand("SET_TRIMS", handleSetTrims);
    SCmd.addCommand("GET_STATUS", handleGetStatus);
    SCmd.addCommand("SERVO_MOVE", handleServoMove);
    SCmd.addCommand("STOP", handleStop);
    SCmd.addDefaultHandler(handleDefault);
    
    // 初始化Otto
    extendedOtto.init();
    
    // 启动完成指示
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    Serial.println(F("STATUS:Otto Serial Control Ready"));
    Serial.println(F("STATUS:Send 'INIT' to initialize robot"));
}

void loop() {
    // 检查命令超时
    if(is_executing_command && (millis() - last_command_time > COMMAND_TIMEOUT)) {
        is_executing_command = false;
        extendedOtto.stopMovement();
        Serial.println(F("ERROR:Timeout"));
    }
    
    // 处理串口命令
    SCmd.readSerial();
    
    // 状态LED心跳
    static unsigned long last_heartbeat = 0;
    if(millis() - last_heartbeat > 2000) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        last_heartbeat = millis();
    }
    
    delay(10);
}
