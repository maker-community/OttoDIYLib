# Otto DIY 机器人 Arduino 库 - 中文说明文档

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![version](https://img.shields.io/badge/version-13.0-blue)

## 项目概述

Otto DIY 是一个开源的双足机器人项目，使用4个舵机进行控制，专为教育和娱乐目的设计。本库包含了Otto机器人的核心功能，支持多种Arduino兼容开发板。

## 硬件兼容性

### 主要支持的开发板
- **Arduino Nano** (主要推荐)
- Arduino Uno
- Arduino Micro
- Arduino Mega
- Arduino Mini
- Arduino Leonardo
- **ESP8266**
- **ESP32** (开发中)

### 硬件接线说明

#### 默认引脚配置 (Arduino AVR系列)
```cpp
#define LeftLeg 2     // 左腿舵机引脚
#define RightLeg 3    // 右腿舵机引脚
#define LeftFoot 4    // 左脚舵机引脚
#define RightFoot 5   // 右脚舵机引脚
#define Buzzer 13     // 蜂鸣器引脚
```

#### LED矩阵显示器接线 (可选)
```cpp
#define DIN A3        // 数据输入引脚
#define CS A2         // 片选引脚
#define CLK A1        // 时钟引脚
```

#### 超声波传感器接线 (可选)
```cpp
#define Trigger 8     // 触发引脚
#define Echo 9        // 回声引脚
```

## 核心库结构

### 主要文件说明

| 文件名 | 功能描述 |
|--------|----------|
| `Otto.h` & `Otto.cpp` | 核心主要功能类 |
| `Otto_gestures.h` | 手势和表情动作定义 |
| `Otto_mouths.h` | LED矩阵嘴部表情 |
| `Otto_sounds.h` | 音调和声音定义 |
| `Otto_matrix.h` | LED矩阵控制 |
| `Oscillator.h` | 舵机平滑运动算法 |
| `SerialCommand.h` | 蓝牙串口通信 |

## 基础使用

### 1. 库导入和初始化

```cpp
#include <Otto.h>
Otto Otto;  // 创建Otto对象

void setup() {
    // 初始化Otto: (左腿, 右腿, 左脚, 右脚, 加载校准, 蜂鸣器)
    Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true, Buzzer);
    Otto.home();  // 回到初始位置
}
```

### 2. LED矩阵初始化 (可选)

```cpp
void setup() {
    Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true, Buzzer);
    Otto.initMATRIX(DIN, CS, CLK, 1);  // 1=顶部朝上
    Otto.matrixIntensity(7);           // 设置亮度 (0-15)
}
```

## 功能详解

## 一、基础动作功能

### 1.1 行走函数
```cpp
Otto.walk(steps, time, direction);
```
- **steps**: 步数
- **time**: 每步持续时间 (毫秒) - 值越大速度越慢
  - 快速: 500ms
  - 正常: 1000ms 
  - 慢速: 2000ms
- **direction**: 方向
  - `1` 或 `FORWARD`: 前进
  - `-1` 或 `BACKWARD`: 后退

**示例:**
```cpp
Otto.walk(4, 1000, 1);  // 前进4步，正常速度
Otto.walk(2, 500, -1);  // 后退2步，快速
```

### 1.2 转向函数
```cpp
Otto.turn(steps, time, direction);
```
- **direction**: 
  - `1` 或 `LEFT`: 左转
  - `-1` 或 `RIGHT`: 右转

**示例:**
```cpp
Otto.turn(3, 1000, 1);   // 左转3步
Otto.turn(2, 1500, -1);  // 右转2步
```

### 1.3 弯腰函数
```cpp
Otto.bend(steps, time, direction);
```
- **direction**: 
  - `1`: 向左弯腰
  - `-1`: 向右弯腰

### 1.4 抬腿函数
```cpp
Otto.shakeLeg(steps, time, direction);
```
- **direction**: 
  - `1`: 抬左腿
  - `-1`: 抬右腿

### 1.5 跳跃函数
```cpp
Otto.jump(steps, time);
```
注意：Otto并不会真正跳跃，这是一个模拟跳跃的动作。

## 二、高级舞蹈动作

### 2.1 上下摆动
```cpp
Otto.updown(steps, time, height);
```
- **height**: 摆动高度 (0-90度，推荐0-50)

### 2.2 左右摇摆
```cpp
Otto.swing(steps, time, height);
```
- **height**: 摇摆幅度 (0-50，推荐20-30)

### 2.3 踮脚摇摆
```cpp
Otto.tiptoeSwing(steps, time, height);
```

### 2.4 抖动
```cpp
Otto.jitter(steps, time, height);
```
- **height**: 抖动幅度 (5-25)

### 2.5 太空步 (模仿迈克尔·杰克逊)
```cpp
Otto.moonwalker(steps, time, height, direction);
```
- **height**: 动作幅度 (15-40)
- **direction**: 
  - `1` 或 `LEFT`: 向左太空步
  - `-1` 或 `RIGHT`: 向右太空步

**示例:**
```cpp
Otto.moonwalker(3, 1000, 25, 1);  // 向左太空步3次
```

### 2.6 混合步态 (Crusaito)
```cpp
Otto.crusaito(steps, time, height, direction);
```
- **height**: 动作高度 (20-50)
- **direction**: 
  - `1` 或 `FORWARD`: 向前
  - `-1` 或 `BACKWARD`: 向后

### 2.7 扇动翅膀
```cpp
Otto.flapping(steps, time, height, direction);
```
- **height**: 扇动幅度 (10-30)
- **direction**: 
  - `1` 或 `FORWARD`: 向前扇动
  - `-1` 或 `BACKWARD`: 向后扇动

### 2.8 上升转向
```cpp
Otto.ascendingTurn(steps, time, height);
```
- **height**: 转向幅度 (5-15)

## 三、声音功能

### 3.1 预设音效
```cpp
Otto.sing(soundName);
```

**可用音效列表:**
- `S_connection` - 连接音
- `S_disconnection` - 断开连接音
- `S_buttonPushed` - 按钮按下音
- `S_mode1`, `S_mode2`, `S_mode3` - 模式切换音
- `S_surprise` - 惊讶音
- `S_OhOoh`, `S_OhOoh2` - 惊叹音
- `S_cuddly` - 可爱音
- `S_sleeping` - 睡觉音
- `S_happy` - 开心音
- `S_superHappy` - 超级开心音
- `S_happy_short` - 短开心音
- `S_sad` - 悲伤音
- `S_confused` - 困惑音
- `S_fart1`, `S_fart2`, `S_fart3` - 放屁音

**示例:**
```cpp
Otto.sing(S_happy);      // 播放开心音
Otto.sing(S_surprise);   // 播放惊讶音
```

### 3.2 自定义音调
```cpp
Otto._tone(frequency, duration, silent);
```
- **frequency**: 音调频率 (Hz)
- **duration**: 持续时间 (毫秒)
- **silent**: 静音间隔 (毫秒)

### 3.3 弯曲音调
```cpp
Otto.bendTones(startFreq, endFreq, proportion, duration, silent);
```
- **startFreq**: 起始频率
- **endFreq**: 结束频率
- **proportion**: 变化比例
- **duration**: 持续时间
- **silent**: 静音间隔

## 四、手势和表情

### 4.1 表情手势
```cpp
Otto.playGesture(gestureName);
```

**可用手势列表:**
- `OttoHappy` - 开心
- `OttoSuperHappy` - 超级开心
- `OttoSad` - 悲伤
- `OttoSleeping` - 睡觉
- `OttoFart` - 放屁
- `OttoConfused` - 困惑
- `OttoLove` - 爱心
- `OttoAngry` - 生气
- `OttoFretful` - 烦躁
- `OttoMagic` - 魔法
- `OttoWave` - 挥手
- `OttoVictory` - 胜利
- `OttoFail` - 失败

**示例:**
```cpp
Otto.playGesture(OttoHappy);    // 表演开心手势
Otto.playGesture(OttoWave);     // 表演挥手手势
```

## 五、LED矩阵显示

### 5.1 嘴部表情显示
```cpp
Otto.putMouth(mouthShape);
```

**预设表情:**
- `smile` - 微笑
- `happyOpen` - 开心张嘴
- `happyClosed` - 开心闭嘴
- `heart` - 爱心
- `angry` - 生气
- `sad` - 悲伤
- `confused` - 困惑
- `zero` - 数字0
- `one` - 数字1
- `two` - 数字2
- ... (数字0-9)

### 5.2 动画嘴部
```cpp
Otto.putAnimationMouth(animationType, frame);
```

**动画类型:**
- `littleUuh` - 小惊讶动画
- `dreamMouth` - 做梦嘴巴动画

### 5.3 自定义LED控制
```cpp
Otto.setLed(x, y, value);     // 设置单个LED
Otto.clearMouth();            // 清除显示
```

### 5.4 滚动文字
```cpp
Otto.writeText("Hello", speed);  // 显示滚动文字
```

## 六、校准和高级功能

### 6.1 舵机校准
```cpp
Otto.setTrims(leftLeg, rightLeg, leftFoot, rightFoot);
Otto.saveTrimsOnEEPROM();  // 保存校准值到EEPROM
```

### 6.2 位置控制
```cpp
Otto.home();  // 回到初始位置

// 检查和设置休息状态
bool isResting = Otto.getRestState();
Otto.setRestState(true);   // 设置为休息状态
Otto.setRestState(false);  // 取消休息状态
```

### 6.3 舵机速度限制
```cpp
Otto.enableServoLimit(240);   // 启用速度限制 (度/秒)
Otto.disableServoLimit();     // 禁用速度限制
```

### 6.4 单独舵机控制
```cpp
Otto._moveSingle(angle, servoNumber);
// servoNumber: 0=左腿, 1=右腿, 2=左脚, 3=右脚
```

## 七、示例项目

### 7.1 基础示例
```cpp
#include <Otto.h>
Otto Otto;

#define LeftLeg 2 
#define RightLeg 3
#define LeftFoot 4 
#define RightFoot 5 
#define Buzzer 13 

void setup() {
    Otto.init(LeftLeg, RightLeg, LeftFoot, RightFoot, true, Buzzer);
    Otto.home();
    Otto.sing(S_connection);
    Otto.playGesture(OttoHappy);
}

void loop() {
    Otto.walk(4, 1000, 1);           // 前进4步
    Otto.turn(2, 1000, 1);           // 左转2步
    Otto.moonwalker(3, 1000, 25, 1); // 太空步
    Otto.playGesture(OttoHappy);     // 开心手势
    delay(1000);
}
```

### 7.2 避障机器人示例
```cpp
#include <Otto.h>
Otto Otto;

#define Trigger 8
#define Echo 9

long getDistance() {
    long duration, distance;
    digitalWrite(Trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger, LOW);
    duration = pulseIn(Echo, HIGH);
    distance = duration / 58;
    return distance;
}

void setup() {
    Otto.init(2, 3, 4, 5, true, 13);
    pinMode(Trigger, OUTPUT);
    pinMode(Echo, INPUT);
    Otto.home();
    Otto.sing(S_happy);
}

void loop() {
    if (getDistance() <= 15) {
        Otto.sing(S_surprise);
        Otto.playGesture(OttoConfused);
        Otto.walk(2, 1000, -1);  // 后退
        Otto.turn(3, 1000, 1);   // 左转
    }
    Otto.walk(1, 1000, 1);       // 前进
}
```

### 7.3 触摸模式切换示例
```cpp
#include <Otto.h>
Otto Otto;

#define TouchSensor A0
int mode = 0;

void setup() {
    Otto.init(2, 3, 4, 5, true, 13);
    pinMode(TouchSensor, INPUT);
    Otto.home();
    Otto.sing(S_happy);
}

void loop() {
    if (digitalRead(TouchSensor) == HIGH) {
        mode = (mode + 1) % 3;  // 循环切换模式
        Otto.sing(S_buttonPushed);
        delay(500);
    }
    
    switch (mode) {
        case 0:  // 避障模式
            // 避障逻辑
            break;
        case 1:  // 跟随模式
            // 跟随逻辑
            break;
        case 2:  // 舞蹈模式
            Otto.jitter(10, 500, 40);
            Otto.moonwalker(2, 1000, 30, 1);
            Otto.swing(3, 1000, 20);
            break;
    }
}
```

## 八、蓝牙控制

Otto支持通过蓝牙模块进行远程控制，配合手机APP使用：

### 8.1 蓝牙初始化
```cpp
#include <SerialCommand.h>
SoftwareSerial BTserial = SoftwareSerial(11, 12);
SerialCommand SCmd(BTserial);

void setup() {
    BTserial.begin(9600);
    // 注册命令处理函数
    SCmd.addCommand("S", receiveStop);
    SCmd.addCommand("M", receiveMovement);
    SCmd.addCommand("H", receiveGesture);
    // ...
}

void loop() {
    SCmd.readSerial();  // 处理蓝牙命令
}
```

## 九、性能优化建议

### 9.1 动作参数调优
- **时间参数(T)**: 
  - 快速动作: 500-800ms
  - 正常动作: 1000-1500ms
  - 慢速动作: 2000-3000ms

- **高度参数(h)**:
  - 小幅动作: 5-15
  - 中等动作: 15-30
  - 大幅动作: 30-50

### 9.2 电源管理
- 使用足够容量的电池 (推荐4节AA或7.4V锂电池)
- 在不需要时调用 `Otto.home()` 让舵机休息
- 合理设置动作间隔，避免过度消耗

### 9.3 内存优化
- ESP8266/ESP32有更多内存，可以运行更复杂的程序
- Arduino Nano内存有限，避免同时使用太多功能

## 十、常见问题解决

### 10.1 舵机校准问题
```cpp
// 校准程序示例
void calibrateServos() {
    Otto.setTrims(0, 0, 0, 0);  // 重置校准
    Otto.home();
    // 手动调整每个舵机的trim值
    // 保存到EEPROM
    Otto.saveTrimsOnEEPROM();
}
```

### 10.2 动作不稳定
- 检查电源是否稳定
- 确认舵机连接牢固
- 调整动作时间参数，增加稳定性

### 10.3 声音问题
- 确认蜂鸣器连接正确
- 检查引脚定义是否匹配
- 测试简单音调：`Otto._tone(440, 1000, 100);`

## 十一、拓展开发

### 11.1 添加新传感器
```cpp
// 添加光线传感器示例
#define LightSensor A0

void setup() {
    Otto.init(2, 3, 4, 5, true, 13);
    pinMode(LightSensor, INPUT);
}

void loop() {
    int lightLevel = analogRead(LightSensor);
    if (lightLevel < 200) {  // 光线较暗
        Otto.playGesture(OttoSleeping);
    } else {
        Otto.playGesture(OttoHappy);
    }
}
```

### 11.2 创建自定义动作
```cpp
void customDance() {
    Otto.walk(2, 800, 1);
    Otto.swing(3, 600, 25);
    Otto.moonwalker(2, 1000, 30, 1);
    Otto.jitter(5, 400, 20);
    Otto.playGesture(OttoVictory);
}
```

### 11.3 数据记录
```cpp
// 记录动作序列到EEPROM
void saveSequence() {
    // 实现动作序列保存逻辑
}

void playSequence() {
    // 实现动作序列回放逻辑
}
```

## 十二、技术规格

### 12.1 硬件要求
- **微控制器**: Arduino兼容 (推荐Arduino Nano)
- **舵机**: 4个9g舵机 (SG90或类似)
- **电源**: 4节AA电池或7.4V锂电池
- **可选组件**: 
  - HC-SR04超声波传感器
  - 8x8 LED矩阵
  - 蜂鸣器
  - 蓝牙模块 (HC-05/HC-06)

### 12.2 软件环境
- **Arduino IDE**: 1.8.0或更高版本
- **支持架构**: AVR, ESP8266, ESP32
- **依赖库**: 
  - Servo库 (Arduino内置)
  - SoftwareSerial库 (Arduino内置)
  - EEPROM库 (Arduino内置)

## 十三、社区与支持

- **官方网站**: https://www.ottodiy.com/
- **GitHub仓库**: https://github.com/OttoDIY/OttoDIYLib
- **许可证**: GPL v3开源许可证
- **社区支持**: Discord服务器

---

## 结语

Otto DIY是一个优秀的教育机器人平台，适合初学者学习机器人编程，也为高级用户提供了丰富的拓展可能性。通过本文档，您应该能够充分理解并使用Otto的各项功能，创造出属于自己的机器人项目。

记住：机器人编程是一个循序渐进的过程，从简单的动作开始，逐步添加更复杂的功能。享受创造的乐趣！

**最后更新日期**: 2025年7月21日
**文档版本**: 1.0
**对应库版本**: OttoDIYLib v13.0
