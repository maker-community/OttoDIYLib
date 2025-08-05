//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-- Otto DIY Serial Control - Arduino Nano Mini Version
//-- 精简版本用于Arduino Nano，减少内存使用
//-- 
//-- 硬件要求:
//-- * Arduino Nano (ATmega328P)
//-- * 6个微型舵机 (SG90或类似)
//-- * 蜂鸣器 (可选)
//-- * 独立5V电源 (舵机用，推荐3-5A)
//-- 
//-- 引脚连接:
//-- * 左腿舵机  -> D3  
//-- * 右腿舵机  -> D5  
//-- * 左脚舵机  -> D6  
//-- * 右脚舵机  -> D9  
//-- * 左手舵机  -> D10 
//-- * 右手舵机  -> D11 
//-- * 蜂鸣器    -> D13 
//-- 
//-- 串口协议格式: COMMAND param1 param2 param3...\n
//-- 波特率: 115200
//-- 
//-- 支持的命令:
//-- INIT - 初始化机器人
//-- HOME hands_down - 回到初始位置
//-- WALK steps speed direction amount - 行走
//-- MOVE_FORWARD steps speed - 前进
//-- MOVE_BACKWARD steps speed - 后退
//-- TURN steps speed direction amount - 转向
//-- TURN_LEFT steps speed - 左转
//-- TURN_RIGHT steps speed - 右转
//-- JUMP steps speed - 跳跃
//-- SWING steps speed height - 摇摆
//-- MOONWALK steps speed height direction - 太空步
//-- HANDS_UP speed direction - 举手
//-- HANDS_DOWN speed direction - 放手
//-- HAND_WAVE speed direction - 挥手
//-- GET_STATUS - 获取状态
//-- SERVO_MOVE servo position - 控制舵机
//-- STOP - 停止
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <SerialCommand.h>
#include <Otto.h>
#include <Servo.h>

// 引脚定义
#define LEFT_LEG_PIN    3
#define RIGHT_LEG_PIN   5
#define LEFT_FOOT_PIN   6
#define RIGHT_FOOT_PIN  9
#define LEFT_HAND_PIN   10
#define RIGHT_HAND_PIN  11
#define BUZZER_PIN      13
#define STATUS_LED_PIN  LED_BUILTIN

// 舵机索引
#define LEFT_LEG    0
#define RIGHT_LEG   1
#define LEFT_FOOT   2
#define RIGHT_FOOT  3
#define LEFT_HAND   4
#define RIGHT_HAND  5

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
bool is_executing = false;
unsigned long cmd_start_time = 0;
const unsigned long CMD_TIMEOUT = 8000; // 8秒超时

// 手部舵机
Servo leftHand, rightHand;
bool has_hands = true;
bool is_resting = false;

// 初始化
void initRobot() {
    ottoRobot.init(LEFT_LEG_PIN, RIGHT_LEG_PIN, LEFT_FOOT_PIN, RIGHT_FOOT_PIN, true, BUZZER_PIN);
    
    if(has_hands) {
        leftHand.attach(LEFT_HAND_PIN);
        rightHand.attach(RIGHT_HAND_PIN);
        leftHand.write(HAND_HOME_POSITION);
        rightHand.write(180 - HAND_HOME_POSITION);
        delay(500);
    }
    
    is_resting = false;
    Serial.println(F("OK:INIT"));
}

// 回到初始位置
void homePosition(bool hands_down = true) {
    ottoRobot.home();
    
    if(has_hands && hands_down) {
        leftHand.write(HAND_HOME_POSITION);
        rightHand.write(180 - HAND_HOME_POSITION);
        delay(500);
    }
    
    is_resting = true;
    Serial.println(F("OK:HOME"));
}

// 走路
void walkRobot(float steps, int speed, int direction, int amount = 0) {
    if(is_resting) is_resting = false;
    
    is_executing = true;
    cmd_start_time = millis();
    
    if(amount > 0 && has_hands) {
        // 带手臂摆动的走路
        int swing_range = constrain(amount, 10, 60);
        int left_center = HAND_HOME_POSITION;
        int right_center = 180 - HAND_HOME_POSITION;
        
        for(int step = 0; step < steps; step++) {
            if(millis() - cmd_start_time > CMD_TIMEOUT) break;
            
            if(step % 2 == 0) {
                leftHand.write(left_center - swing_range/2);
                rightHand.write(right_center + swing_range/2);
            } else {
                leftHand.write(left_center + swing_range/2);
                rightHand.write(right_center - swing_range/2);
            }
            
            ottoRobot.walk(1, speed, direction);
            delay(50);
        }
        
        leftHand.write(left_center);
        rightHand.write(right_center);
    } else {
        ottoRobot.walk(steps, speed, direction);
    }
    
    is_executing = false;
    Serial.println(F("OK:WALK"));
}

// 转向
void turnRobot(float steps, int speed, int direction, int amount = 0) {
    if(is_resting) is_resting = false;
    
    is_executing = true;
    cmd_start_time = millis();
    
    ottoRobot.turn(steps, speed, direction);
    
    is_executing = false;
    Serial.println(F("OK:TURN"));
}

// 手部动作
void handsUp(int speed = 1000, int direction = 0) {
    if(!has_hands) {
        Serial.println(F("ERROR:No hands"));
        return;
    }
    
    if(direction == 0 || direction == 1) {
        leftHand.write(170);
    }
    if(direction == 0 || direction == -1) {
        rightHand.write(10);
    }
    
    Serial.println(F("OK:HANDS_UP"));
}

void handsDown(int speed = 1000, int direction = 0) {
    if(!has_hands) {
        Serial.println(F("ERROR:No hands"));
        return;
    }
    
    if(direction == 0 || direction == 1) {
        leftHand.write(HAND_HOME_POSITION);
    }
    if(direction == 0 || direction == -1) {
        rightHand.write(180 - HAND_HOME_POSITION);
    }
    
    Serial.println(F("OK:HANDS_DOWN"));
}

void handWave(int speed = 1000, int direction = LEFT) {
    if(!has_hands) {
        Serial.println(F("ERROR:No hands"));
        return;
    }
    
    // 举起手臂
    if(direction == LEFT || direction == 0) {
        leftHand.write(170);
    }
    if(direction == RIGHT || direction == 0) {
        rightHand.write(10);
    }
    delay(speed/3);
    
    // 挥手动作
    for(int i = 0; i < 3; i++) {
        if(direction == LEFT || direction == 0) {
            leftHand.write(140);
            delay(speed/6);
            leftHand.write(180);
            delay(speed/6);
        }
        if(direction == RIGHT || direction == 0) {
            rightHand.write(40);
            delay(speed/6);
            rightHand.write(0);
            delay(speed/6);
        }
    }
    
    // 回到默认位置
    handsDown(speed/2, direction);
    Serial.println(F("OK:HAND_WAVE"));
}

// 串口命令处理函数
void handleInit() {
    initRobot();
}

void handleHome() {
    char *arg = SCmd.next();
    bool hands_down = (arg != NULL) ? (atoi(arg) == 1) : true;
    homePosition(hands_down);
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
    
    walkRobot(steps, speed, direction, amount);
}

void handleMoveForward() {
    float steps = 3;
    int speed = 1200;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    walkRobot(steps, speed, FORWARD, 30);
}

void handleMoveBackward() {
    float steps = 3;
    int speed = 1200;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    walkRobot(steps, speed, BACKWARD, 30);
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
    
    turnRobot(steps, speed, direction, amount);
}

void handleTurnLeft() {
    float steps = 2;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    turnRobot(steps, speed, LEFT, 0);
}

void handleTurnRight() {
    float steps = 2;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    turnRobot(steps, speed, RIGHT, 0);
}

void handleJump() {
    float steps = 1;
    int speed = 2000;
    
    char *arg = SCmd.next();
    if(arg != NULL) steps = atof(arg);
    
    arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    is_executing = true;
    cmd_start_time = millis();
    
    ottoRobot.jump(steps, speed);
    
    is_executing = false;
    Serial.println(F("OK:JUMP"));
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
    
    is_executing = true;
    cmd_start_time = millis();
    
    ottoRobot.swing(steps, speed, height);
    
    is_executing = false;
    Serial.println(F("OK:SWING"));
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
    
    is_executing = true;
    cmd_start_time = millis();
    
    ottoRobot.moonwalker(steps, speed, height, direction);
    
    is_executing = false;
    Serial.println(F("OK:MOONWALK"));
}

void handleHandsUp() {
    int speed = 1000;
    int direction = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    handsUp(speed, direction);
}

void handleHandsDown() {
    int speed = 1000;
    int direction = 0;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    handsDown(speed, direction);
}

void handleHandWave() {
    int speed = 1000;
    int direction = LEFT;
    
    char *arg = SCmd.next();
    if(arg != NULL) speed = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) direction = atoi(arg);
    
    handWave(speed, direction);
}

void handleGetStatus() {
    Serial.print(F("STATUS:rest="));
    Serial.print(is_resting ? F("1") : F("0"));
    Serial.print(F(",hands="));
    Serial.print(has_hands ? F("1") : F("0"));
    Serial.print(F(",exec="));
    Serial.print(is_executing ? F("1") : F("0"));
    Serial.println();
}

void handleServoMove() {
    int servo = -1;
    int position = 90;
    
    char *arg = SCmd.next();
    if(arg != NULL) servo = atoi(arg);
    
    arg = SCmd.next();
    if(arg != NULL) position = atoi(arg);
    
    if(servo >= 0 && servo <= 5 && position >= 0 && position <= 180) {
        if(servo == LEFT_HAND && leftHand.attached()) {
            leftHand.write(position);
        } else if(servo == RIGHT_HAND && rightHand.attached()) {
            rightHand.write(position);
        } else if(servo < 4) {
            ottoRobot._moveSingle(position, servo);
        }
        Serial.println(F("OK:SERVO_MOVE"));
    } else {
        Serial.println(F("ERROR:Invalid servo"));
    }
}

void handleStop() {
    is_executing = false;
    homePosition(false);
    Serial.println(F("OK:STOP"));
}

void handleDefault() {
    Serial.println(F("ERROR:Unknown"));
}

void setup() {
    Serial.begin(115200);
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // 注册命令
    SCmd.addCommand("INIT", handleInit);
    SCmd.addCommand("HOME", handleHome);
    SCmd.addCommand("WALK", handleWalk);
    SCmd.addCommand("MOVE_FORWARD", handleMoveForward);
    SCmd.addCommand("MOVE_BACKWARD", handleMoveBackward);
    SCmd.addCommand("TURN", handleTurn);
    SCmd.addCommand("TURN_LEFT", handleTurnLeft);
    SCmd.addCommand("TURN_RIGHT", handleTurnRight);
    SCmd.addCommand("JUMP", handleJump);
    SCmd.addCommand("SWING", handleSwing);
    SCmd.addCommand("MOONWALK", handleMoonwalk);
    SCmd.addCommand("HANDS_UP", handleHandsUp);
    SCmd.addCommand("HANDS_DOWN", handleHandsDown);
    SCmd.addCommand("HAND_WAVE", handleHandWave);
    SCmd.addCommand("GET_STATUS", handleGetStatus);
    SCmd.addCommand("SERVO_MOVE", handleServoMove);
    SCmd.addCommand("STOP", handleStop);
    SCmd.addDefaultHandler(handleDefault);
    
    // 初始化Otto
    initRobot();
    
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    Serial.println(F("STATUS:Otto Mini Ready"));
}

void loop() {
    // 检查命令超时
    if(is_executing && (millis() - cmd_start_time > CMD_TIMEOUT)) {
        is_executing = false;
        homePosition(false);
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
