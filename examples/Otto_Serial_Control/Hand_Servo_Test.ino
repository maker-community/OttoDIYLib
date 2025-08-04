//--------------------------------------------------------------------------------------------------------------------------------------------------------------------
//-- 简单手臂舵机测试程序
//-- 用于独立测试和验证手臂舵机功能
//-- 在解决Otto_Serial_Control.ino的手臂舵机问题时使用
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <Servo.h>

// 引脚定义
#define LEFT_HAND_PIN   10  // 左手舵机
#define RIGHT_HAND_PIN  11  // 右手舵机
#define STATUS_LED_PIN  LED_BUILTIN

// 舵机对象
Servo leftHandServo;
Servo rightHandServo;

// 测试参数
int testDelay = 1000;  // 测试动作间隔时间
bool testRunning = true;

void setup() {
    // 初始化串口
    Serial.begin(115200);
    Serial.println("Hand Servo Test Started");
    
    // 初始化状态LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // 连接舵机
    leftHandServo.attach(LEFT_HAND_PIN);
    rightHandServo.attach(RIGHT_HAND_PIN);
    
    // 检查舵机连接状态
    if(leftHandServo.attached()) {
        Serial.println("Left hand servo attached successfully");
    } else {
        Serial.println("ERROR: Left hand servo failed to attach");
    }
    
    if(rightHandServo.attached()) {
        Serial.println("Right hand servo attached successfully");
    } else {
        Serial.println("ERROR: Right hand servo failed to attach");
    }
    
    // 初始位置
    Serial.println("Moving to initial position...");
    leftHandServo.write(45);   // 左手默认位置
    rightHandServo.write(135); // 右手默认位置
    delay(1000);
    
    Serial.println("Test ready. Send commands:");
    Serial.println("  '1' - Basic position test");
    Serial.println("  '2' - Sweep test");
    Serial.println("  '3' - Wave test");
    Serial.println("  '4' - Custom position (format: L<pos> R<pos>)");
    Serial.println("  's' - Stop/Resume automatic test");
    Serial.println("  'h' - Help");
    
    digitalWrite(STATUS_LED_PIN, LOW);
}

void loop() {
    // 处理串口命令
    if(Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        handleCommand(command);
    }
    
    // 自动测试（如果启用）
    if(testRunning) {
        runAutomaticTest();
    }
    
    // 状态LED心跳
    static unsigned long lastHeartbeat = 0;
    if(millis() - lastHeartbeat > 2000) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        lastHeartbeat = millis();
    }
    
    delay(50);
}

void handleCommand(String cmd) {
    cmd.toLowerCase();
    
    if(cmd == "1") {
        basicPositionTest();
    } else if(cmd == "2") {
        sweepTest();
    } else if(cmd == "3") {
        waveTest();
    } else if(cmd.startsWith("l") && cmd.indexOf("r") > 0) {
        customPositionTest(cmd);
    } else if(cmd == "s") {
        testRunning = !testRunning;
        Serial.println(testRunning ? "Automatic test resumed" : "Automatic test stopped");
    } else if(cmd == "h") {
        printHelp();
    } else {
        Serial.println("Unknown command. Send 'h' for help.");
    }
}

void basicPositionTest() {
    Serial.println("=== Basic Position Test ===");
    
    // 测试几个基本位置
    int positions[] = {0, 45, 90, 135, 180};
    int numPositions = sizeof(positions) / sizeof(positions[0]);
    
    for(int i = 0; i < numPositions; i++) {
        Serial.print("Position ");
        Serial.print(positions[i]);
        Serial.println(" degrees");
        
        leftHandServo.write(positions[i]);
        rightHandServo.write(180 - positions[i]); // 右手镜像
        
        delay(testDelay);
    }
    
    // 回到默认位置
    Serial.println("Returning to default position");
    leftHandServo.write(45);
    rightHandServo.write(135);
    delay(testDelay);
}

void sweepTest() {
    Serial.println("=== Sweep Test ===");
    
    // 左手向上扫描
    Serial.println("Left hand sweep up");
    for(int pos = 45; pos <= 180; pos += 5) {
        leftHandServo.write(pos);
        delay(50);
    }
    
    // 左手向下扫描
    Serial.println("Left hand sweep down");
    for(int pos = 180; pos >= 45; pos -= 5) {
        leftHandServo.write(pos);
        delay(50);
    }
    
    // 右手向上扫描
    Serial.println("Right hand sweep up");
    for(int pos = 135; pos >= 0; pos -= 5) {
        rightHandServo.write(pos);
        delay(50);
    }
    
    // 右手向下扫描
    Serial.println("Right hand sweep down");
    for(int pos = 0; pos <= 135; pos += 5) {
        rightHandServo.write(pos);
        delay(50);
    }
    
    Serial.println("Sweep test completed");
}

void waveTest() {
    Serial.println("=== Wave Test ===");
    
    // 左手挥手
    Serial.println("Left hand wave");
    for(int i = 0; i < 3; i++) {
        leftHandServo.write(120);
        delay(300);
        leftHandServo.write(60);
        delay(300);
    }
    leftHandServo.write(45); // 回到默认位置
    
    delay(500);
    
    // 右手挥手
    Serial.println("Right hand wave");
    for(int i = 0; i < 3; i++) {
        rightHandServo.write(60);
        delay(300);
        rightHandServo.write(120);
        delay(300);
    }
    rightHandServo.write(135); // 回到默认位置
    
    delay(500);
    
    // 双手同时挥手
    Serial.println("Both hands wave");
    for(int i = 0; i < 3; i++) {
        leftHandServo.write(120);
        rightHandServo.write(60);
        delay(300);
        leftHandServo.write(60);
        rightHandServo.write(120);
        delay(300);
    }
    
    // 回到默认位置
    leftHandServo.write(45);
    rightHandServo.write(135);
    
    Serial.println("Wave test completed");
}

void customPositionTest(String cmd) {
    // 解析命令格式: L<pos> R<pos>
    int lPos = -1, rPos = -1;
    
    int lIndex = cmd.indexOf('l');
    int rIndex = cmd.indexOf('r');
    
    if(lIndex >= 0 && rIndex > lIndex) {
        String lStr = cmd.substring(lIndex + 1, rIndex);
        String rStr = cmd.substring(rIndex + 1);
        
        lPos = lStr.toInt();
        rPos = rStr.toInt();
    }
    
    if(lPos >= 0 && lPos <= 180 && rPos >= 0 && rPos <= 180) {
        Serial.print("Custom position: Left=");
        Serial.print(lPos);
        Serial.print(", Right=");
        Serial.println(rPos);
        
        leftHandServo.write(lPos);
        rightHandServo.write(rPos);
    } else {
        Serial.println("Invalid position. Format: L<0-180> R<0-180>");
        Serial.println("Example: L90 R90");
    }
}

void runAutomaticTest() {
    static unsigned long lastTest = 0;
    static int testPhase = 0;
    
    if(millis() - lastTest > 5000) { // 每5秒执行一次测试
        lastTest = millis();
        
        switch(testPhase) {
            case 0:
                Serial.println("Auto test: Basic positions");
                basicPositionTest();
                break;
            case 1:
                Serial.println("Auto test: Wave test");
                waveTest();
                break;
            case 2:
                Serial.println("Auto test: Sweep test");
                sweepTest();
                break;
        }
        
        testPhase = (testPhase + 1) % 3;
    }
}

void printHelp() {
    Serial.println("=== Hand Servo Test Commands ===");
    Serial.println("1 - Basic position test (0, 45, 90, 135, 180 degrees)");
    Serial.println("2 - Sweep test (smooth movement across range)");
    Serial.println("3 - Wave test (simulate waving motion)");
    Serial.println("4 - Custom position (format: L<pos> R<pos>, e.g., L90 R90)");
    Serial.println("s - Stop/Resume automatic test");
    Serial.println("h - Show this help");
    Serial.println();
    Serial.println("Current status:");
    Serial.print("- Left hand servo: ");
    Serial.println(leftHandServo.attached() ? "Attached" : "Not attached");
    Serial.print("- Right hand servo: ");
    Serial.println(rightHandServo.attached() ? "Attached" : "Not attached");
    Serial.print("- Automatic test: ");
    Serial.println(testRunning ? "Running" : "Stopped");
}
