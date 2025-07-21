# Otto Serial Control - Arduino Nano版本

这是一个基于xiaozhi-esp32 otto-robot功能的Arduino Nano串口控制实现，允许通过串口命令控制Otto机器人的所有动作。

## 功能特性

### 🤖 支持的动作
- **基础移动**: 行走(前后)、转向(左右)、跳跃
- **特殊动作**: 摇摆、太空步、弯曲身体、摇腿、上下运动、踮脚摇摆、抖动、上升转向、十字步、拍打
- **手部动作**: 举手、放手、挥手 (支持6舵机版本)
- **舵机控制**: 单独控制每个舵机
- **系统功能**: 舵机微调、速度限制、状态查询

### 📡 通信协议
- **波特率**: 115200
- **格式**: `COMMAND:param1,param2,param3,...\\n`
- **响应**: `OK:command` 或 `ERROR:message` 或 `STATUS:info`

## 硬件配置

### Arduino Nano 引脚连接

#### 舵机连接 (优化PWM分配)
| 舵机部位 | Arduino引脚 | 引脚类型 | 说明 |
|----------|-------------|----------|------|
| 左腿     | D3          | PWM      | 支持PWM，舵机控制最佳 |
| 右腿     | D5          | PWM      | 支持PWM，舵机控制最佳 |
| 左脚     | D6          | PWM      | 支持PWM，舵机控制最佳 |
| 右脚     | D9          | PWM      | 支持PWM，舵机控制最佳 |
| 左手     | D10         | PWM      | 支持PWM，舵机控制最佳 |
| 右手     | D11         | PWM      | 支持PWM，舵机控制最佳 |

#### 其他组件
| 组件     | Arduino引脚 | 说明 |
|----------|-------------|------|
| 蜂鸣器   | D13         | 内置LED引脚，可共用 |
| 状态LED  | D13         | 内置LED，状态指示 |
| 串口通信 | D0(RX), D1(TX) | 硬件串口，与ESP32通信 |

> 📖 **详细引脚指南**: 查看 [PINOUT_GUIDE.md](PINOUT_GUIDE.md) 获取完整的引脚布局和连接说明

### 电源要求
- **Arduino**: 通过USB或VIN供电 (7-12V)
- **舵机**: 独立5V电源 (推荐5V/3-5A)
- **重要**: 确保Arduino和舵机电源共地

> 📋 **硬件连接指南**: 查看 [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) 获取完整的连接图和步骤说明

## 文档索引

| 文档 | 内容 |
|------|------|
| [README.md](README.md) | 主要说明文档 |
| [PINOUT_GUIDE.md](PINOUT_GUIDE.md) | Arduino Nano 引脚对应详细指南 |
| [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) | 硬件连接图和步骤说明 |
| [CONNECTION_GUIDE.md](CONNECTION_GUIDE.md) | ESP32集成连接指南 |
D2      左腿舵机    Left Leg Servo
D3      右腿舵机    Right Leg Servo  
D4      左脚舵机    Left Foot Servo
D5      右脚舵机    Right Foot Servo
D6      左手舵机    Left Hand Servo (可选)
D7      右手舵机    Right Hand Servo (可选)
D13     蜂鸣器      Buzzer
LED     状态指示    内置LED心跳指示
```

### 舵机索引
```
索引  位置        说明
0     左腿        Left Leg
1     右腿        Right Leg
2     左脚        Left Foot  
3     右脚        Right Foot
4     左手        Left Hand
5     右手        Right Hand
```

## 安装配置

### 1. 硬件准备
- Arduino Nano (或兼容板)
- 6个舵机 (SG90或类似)
- 蜂鸣器 (可选)
- 电源 (建议外部电源给舵机供电)

### 2. 软件环境
需要安装以下Arduino库:
- **OttoDIYLib**: Otto机器人核心库
- **SerialCommand**: 串口命令解析库
- **Servo**: Arduino标准舵机库

### 3. 编译上传
1. 在Arduino IDE中打开 `Otto_Serial_Control.ino`
2. 选择正确的板卡和端口
3. 编译并上传到Arduino Nano

## 串口命令参考

### 基础命令

#### INIT
初始化机器人
```
INIT
```

#### HOME
回到初始位置
```
HOME:hands_down
```
- `hands_down`: 0=保持手部位置, 1=放下手臂

#### GET_STATUS  
获取机器人状态
```
GET_STATUS
```

#### STOP
停止当前动作
```
STOP
```

### 移动命令

#### WALK
行走动作
```
WALK:steps,speed,direction,amount
```
- `steps`: 步数 (float)
- `speed`: 速度/周期 (ms)
- `direction`: 方向 (1=前进, -1=后退)
- `amount`: 手臂摆动幅度 (0=不摆动)

示例:
```
WALK:2,1000,1,30    # 前进2步，速度1000ms，手臂摆动幅度30
WALK:1,1200,-1,0    # 后退1步，速度1200ms，不摆动手臂
```

#### TURN
转向动作
```
TURN:steps,speed,direction,amount
```
- `steps`: 步数 (float)
- `speed`: 速度/周期 (ms)  
- `direction`: 方向 (1=左转, -1=右转)
- `amount`: 手臂摆动幅度

示例:
```
TURN:1,2000,1,0     # 左转1步，速度2000ms
TURN:2,1500,-1,20   # 右转2步，速度1500ms，手臂摆动
```

#### JUMP
跳跃动作
```
JUMP:steps,speed
```
- `steps`: 跳跃次数 (float)
- `speed`: 速度/周期 (ms)

示例:
```
JUMP:1,2000         # 跳跃1次，速度2000ms
JUMP:3,1500         # 连续跳跃3次
```

### 特殊动作

#### SWING
摇摆动作
```
SWING:steps,speed,height
```
- `steps`: 摇摆次数 (float)
- `speed`: 速度/周期 (ms)
- `height`: 摇摆幅度 (度)

#### MOONWALK
太空步
```
MOONWALK:steps,speed,height,direction
```
- `steps`: 步数 (float)
- `speed`: 速度/周期 (ms)
- `height`: 抬腿高度 (度)
- `direction`: 方向 (1=左, -1=右)

#### BEND
弯曲动作
```
BEND:steps,speed,direction
```
- `steps`: 弯曲次数 (float)
- `speed`: 速度/周期 (ms)
- `direction`: 方向 (1=左, -1=右)

#### SHAKE_LEG
摇腿动作
```
SHAKE_LEG:steps,speed,direction
```
- `steps`: 摇腿次数 (float)
- `speed`: 速度/周期 (ms)
- `direction`: 腿部方向 (1=左腿, -1=右腿)

#### UPDOWN
上下运动
```
UPDOWN:steps,speed,height
```
- `steps`: 运动次数 (float)
- `speed`: 速度/周期 (ms)
- `height`: 运动幅度 (度)

#### TIPTOE_SWING
踮脚摇摆
```
TIPTOE_SWING:steps,speed,height
```

#### JITTER
抖动
```
JITTER:steps,speed,height
```

#### ASCENDING_TURN
上升转向
```
ASCENDING_TURN:steps,speed,height
```

#### CRUSAITO
十字步
```
CRUSAITO:steps,speed,height,direction
```
- `direction`: 方向 (1=前进, -1=后退)

#### FLAPPING
拍打动作
```
FLAPPING:steps,speed,height,direction
```
- `direction`: 方向 (1=前进, -1=后退)

### 手部动作 (仅6舵机版本)

#### HANDS_UP
举手
```
HANDS_UP:speed,direction
```
- `speed`: 动作速度 (ms)
- `direction`: 方向 (0=双手, 1=左手, -1=右手)

#### HANDS_DOWN
放手
```
HANDS_DOWN:speed,direction
```

#### HAND_WAVE
挥手
```
HAND_WAVE:speed,direction
```
- `direction`: 方向 (1=左手, -1=右手, 0=双手)

### 系统控制

#### SET_TRIMS
设置舵机微调
```
SET_TRIMS:yl,yr,rl,rr,lh,rh
```
- `yl`: 左腿微调 (-90 to 90)
- `yr`: 右腿微调
- `rl`: 左脚微调  
- `rr`: 右脚微调
- `lh`: 左手微调
- `rh`: 右手微调

#### ENABLE_LIMIT
启用舵机速度限制
```
ENABLE_LIMIT:speed_limit
```
- `speed_limit`: 速度限制 (度/秒，默认240)

#### DISABLE_LIMIT
禁用舵机速度限制
```
DISABLE_LIMIT
```

#### SERVO_MOVE
单独控制舵机
```
SERVO_MOVE:servo,position
```
- `servo`: 舵机索引 (0-5)
- `position`: 目标位置 (0-180度)

## 使用示例

### Python测试脚本
项目包含一个Python测试脚本 `otto_serial_test.py`，提供以下功能：
- 基础动作演示
- 手部动作演示  
- 高级动作演示
- 交互模式

#### 运行测试脚本
```bash
# 修改脚本中的COM_PORT为实际串口
python otto_serial_test.py
```

#### 交互模式示例
```
Otto> INIT
响应: OK:INIT

Otto> WALK:2,1000,1,30
响应: OK:WALK

Otto> HAND_WAVE:1000,1
响应: OK:HAND_WAVE

Otto> GET_STATUS
响应: STATUS:resting=false,has_hands=true
```

### 其他编程语言
任何支持串口通信的编程语言都可以控制Otto机器人:

#### C# 示例
```csharp
SerialPort port = new SerialPort("COM3", 115200);
port.Open();
port.WriteLine("WALK:2,1000,1,30");
string response = port.ReadLine();
```

#### Node.js 示例
```javascript
const SerialPort = require('serialport');
const port = new SerialPort('COM3', { baudRate: 115200 });

port.write('WALK:2,1000,1,30\\n');
port.on('data', (data) => {
    console.log('响应:', data.toString());
});
```

## 性能参数建议

### 动作速度设置
- **低速动作**: 1200-1500ms (适合精确控制)
- **中速动作**: 900-1200ms (日常使用推荐)  
- **高速动作**: 500-800ms (表演和娱乐)

### 动作幅度设置
- **小幅度**: 10-30 (细腻动作)
- **中幅度**: 30-60 (标准动作)
- **大幅度**: 60-120 (夸张表演)

## 故障排除

### 常见问题

#### 1. 串口连接失败
- 检查COM端口是否正确
- 确认Arduino已正确连接并上传代码
- 检查波特率设置 (115200)

#### 2. 舵机不动作
- 检查电源供电是否充足
- 确认舵机连接线路
- 验证引脚定义是否正确

#### 3. 动作不协调
- 使用SET_TRIMS命令校准舵机
- 检查舵机安装是否正确
- 调整动作参数

#### 4. 通信超时
- 增加串口超时时间
- 检查命令格式是否正确
- 确认换行符设置

### 调试技巧

#### 1. 查看状态
```
GET_STATUS
```

#### 2. 测试单个舵机
```
SERVO_MOVE:0,90    # 测试左腿舵机
SERVO_MOVE:1,90    # 测试右腿舵机
```

#### 3. 回到安全位置
```
HOME:1             # 强制回到初始位置
```

## 扩展开发

### MCP协议集成
这个串口控制系统可以很容易地集成到支持MCP协议的系统中，如xiaozhi-esp32项目的sparkbot方式。

### 自定义动作
可以通过组合基础命令创建自定义动作序列：
```python
# 自定义舞蹈动作
commands = [
    "HANDS_UP:1000,0",
    "SWING:2,1000,30", 
    "TURN:1,2000,1,0",
    "JUMP:1,1500",
    "HAND_WAVE:1000,0",
    "HOME:1"
]

for cmd in commands:
    otto.send_command(cmd)
    time.sleep(2)
```

### 添加新命令
在Arduino代码中添加新的命令处理函数：
```cpp
void handleNewCommand() {
    // 解析参数
    char *arg = SCmd.next();
    
    // 实现新功能
    // ...
    
    Serial.println("OK:NEW_COMMAND");
}

// 在setup()中注册
SCmd.addCommand("NEW_COMMAND", handleNewCommand);
```

## 许可证

本项目基于OttoDIY开源项目，遵循相同的开源许可证。

## 贡献

欢迎提交Issue和Pull Request来改进这个项目！

---

**注意**: 这是一个示例实现，实际使用时可能需要根据具体硬件配置进行调整。
