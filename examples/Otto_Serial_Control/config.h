/*
 * Otto Serial Control Configuration
 * 配置文件 - 根据具体硬件修改此文件
 */

#ifndef OTTO_CONFIG_H
#define OTTO_CONFIG_H

// ===========================================
// 硬件配置选择
// ===========================================

// 舵机数量配置
#define SERVO_4_MODE  0  // 4舵机模式 (腿部+脚部)
#define SERVO_6_MODE  1  // 6舵机模式 (腿部+脚部+手部)

// 当前配置 (修改这里选择硬件配置)
#define CURRENT_MODE SERVO_6_MODE

// ===========================================
// 引脚定义 - Arduino Nano
// ===========================================

// 基础舵机引脚 (所有配置都需要)
#define LEFT_LEG_PIN    2   // 左腿舵机
#define RIGHT_LEG_PIN   3   // 右腿舵机  
#define LEFT_FOOT_PIN   4   // 左脚舵机
#define RIGHT_FOOT_PIN  5   // 右脚舵机

// 手部舵机引脚 (仅6舵机模式)
#if CURRENT_MODE == SERVO_6_MODE
  #define LEFT_HAND_PIN   6   // 左手舵机
  #define RIGHT_HAND_PIN  7   // 右手舵机
#else
  #define LEFT_HAND_PIN   -1  // 禁用左手舵机
  #define RIGHT_HAND_PIN  -1  // 禁用右手舵机
#endif

// 其他组件引脚
#define BUZZER_PIN      13  // 蜂鸣器
#define STATUS_LED_PIN  LED_BUILTIN // 状态LED

// ===========================================
// 舵机参数配置
// ===========================================

// 舵机索引定义
#define LEFT_LEG    0
#define RIGHT_LEG   1  
#define LEFT_FOOT   2
#define RIGHT_FOOT  3
#define LEFT_HAND   4
#define RIGHT_HAND  5
#define SERVO_COUNT 6

// 默认舵机微调值 (可通过SET_TRIMS命令调整)
#define DEFAULT_LEFT_LEG_TRIM    0
#define DEFAULT_RIGHT_LEG_TRIM   0
#define DEFAULT_LEFT_FOOT_TRIM   0
#define DEFAULT_RIGHT_FOOT_TRIM  0
#define DEFAULT_LEFT_HAND_TRIM   0
#define DEFAULT_RIGHT_HAND_TRIM  0

// 手部舵机默认位置
#define HAND_HOME_POSITION 45

// 舵机速度限制 (度/秒)
#define DEFAULT_SERVO_SPEED_LIMIT 240

// ===========================================
// 通信配置
// ===========================================

// 串口配置
#define SERIAL_BAUDRATE 115200
#define SERIAL_TIMEOUT  2000

// 命令缓冲区大小
#define CMD_BUFFER_SIZE 64

// ===========================================
// 动作参数默认值
// ===========================================

// 速度/周期默认值 (毫秒)
#define DEFAULT_WALK_SPEED      1000
#define DEFAULT_TURN_SPEED      2000  
#define DEFAULT_JUMP_SPEED      2000
#define DEFAULT_SWING_SPEED     1000
#define DEFAULT_MOONWALK_SPEED  900
#define DEFAULT_BEND_SPEED      1400
#define DEFAULT_SHAKE_SPEED     2000
#define DEFAULT_UPDOWN_SPEED    1000
#define DEFAULT_TIPTOE_SPEED    900
#define DEFAULT_JITTER_SPEED    500
#define DEFAULT_ASCENDING_SPEED 900
#define DEFAULT_CRUSAITO_SPEED  900
#define DEFAULT_FLAPPING_SPEED  1000
#define DEFAULT_HAND_SPEED      1000

// 幅度默认值 (度)
#define DEFAULT_MOVEMENT_HEIGHT 20
#define DEFAULT_HAND_AMPLITUDE  30

// 步数默认值
#define DEFAULT_WALK_STEPS      1.0
#define DEFAULT_TURN_STEPS      1.0
#define DEFAULT_JUMP_STEPS      1.0
#define DEFAULT_MOVEMENT_STEPS  1.0

// ===========================================
// 高级配置
// ===========================================

// 状态LED心跳间隔 (毫秒)
#define HEARTBEAT_INTERVAL 2000

// 舵机运动插值间隔 (毫秒)
#define SERVO_UPDATE_INTERVAL 10

// 命令执行超时 (毫秒)
#define COMMAND_TIMEOUT 10000

// 调试输出开关
#define DEBUG_OUTPUT 1

// ===========================================
// 硬件特定配置
// ===========================================

// Arduino Nano特定配置
#ifdef ARDUINO_AVR_NANO
  #define BOARD_NAME "Arduino Nano"
  #define MAX_SERVO_COUNT 6
  #define HAS_EEPROM 1
#endif

// Arduino Uno特定配置  
#ifdef ARDUINO_AVR_UNO
  #define BOARD_NAME "Arduino Uno"
  #define MAX_SERVO_COUNT 6
  #define HAS_EEPROM 1
#endif

// ESP32特定配置
#ifdef ARDUINO_ARCH_ESP32
  #define BOARD_NAME "ESP32"
  #define MAX_SERVO_COUNT 16
  #define HAS_EEPROM 0
  #include <EEPROM.h>
#endif

// ===========================================
// 功能开关
// ===========================================

// 启用功能
#define ENABLE_BUZZER_SOUNDS    1  // 启用蜂鸣器声音
#define ENABLE_STATUS_LED       1  // 启用状态LED
#define ENABLE_SERIAL_ECHO      0  // 启用串口回显
#define ENABLE_MOVEMENT_SOUNDS  0  // 启用运动时声音
#define ENABLE_TRIM_SAVE        1  // 启用微调值保存到EEPROM

// 安全功能
#define ENABLE_SERVO_TIMEOUT    1  // 启用舵机超时保护
#define ENABLE_POSITION_LIMITS  1  // 启用位置限制
#define ENABLE_SPEED_LIMITS     1  // 启用速度限制

// ===========================================
// 错误处理配置
// ===========================================

// 错误代码定义
#define ERROR_NONE              0
#define ERROR_INVALID_COMMAND   1
#define ERROR_INVALID_PARAM     2
#define ERROR_SERVO_TIMEOUT     3
#define ERROR_POSITION_LIMIT    4
#define ERROR_HARDWARE_FAULT    5

// 最大重试次数
#define MAX_RETRY_COUNT 3

// ===========================================
// 版本信息
// ===========================================

#define FIRMWARE_VERSION "1.0.0"
#define PROTOCOL_VERSION "1.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

#endif // OTTO_CONFIG_H
