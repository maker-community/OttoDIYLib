#!/usr/bin/env python3
"""
Otto Serial Control Test - 改进版
测试手臂舵机功能
"""

import serial
import time
import sys

class OttoSerialTester:
    def __init__(self, port='COM3', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        
    def connect(self):
        """连接到Arduino"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=2)
            time.sleep(2)  # 等待Arduino重启
            print(f"Connected to {self.port} at {self.baudrate} baud")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False
    
    def send_command(self, command):
        """发送命令并获取响应"""
        if not self.ser:
            print("Not connected!")
            return None
            
        try:
            # 发送命令
            self.ser.write((command + '\n').encode())
            print(f">> {command}")
            
            # 读取响应
            response = self.ser.readline().decode().strip()
            print(f"<< {response}")
            return response
            
        except Exception as e:
            print(f"Command failed: {e}")
            return None
    
    def test_basic_functions(self):
        """测试基础功能"""
        print("\n=== 测试基础功能 ===")
        
        # 初始化
        self.send_command("INIT")
        time.sleep(1)
        
        # 获取状态
        self.send_command("GET_STATUS")
        time.sleep(0.5)
        
        # 回到初始位置
        self.send_command("HOME 1")
        time.sleep(2)
    
    def test_hand_servos(self):
        """测试手臂舵机"""
        print("\n=== 测试手臂舵机 ===")
        
        # 测试单个舵机控制
        print("测试左手舵机 (舵机4)...")
        self.send_command("SERVO_MOVE 4 90")   # 左手中位
        time.sleep(1)
        self.send_command("SERVO_MOVE 4 45")   # 左手下位
        time.sleep(1)
        self.send_command("SERVO_MOVE 4 135")  # 左手上位
        time.sleep(1)
        
        print("测试右手舵机 (舵机5)...")
        self.send_command("SERVO_MOVE 5 90")   # 右手中位
        time.sleep(1)
        self.send_command("SERVO_MOVE 5 135")  # 右手下位
        time.sleep(1)
        self.send_command("SERVO_MOVE 5 45")   # 右手上位
        time.sleep(1)
        
        # 复位到默认位置
        self.send_command("HANDS_DOWN 1000 0")
        time.sleep(2)
    
    def test_hand_actions(self):
        """测试手部动作"""
        print("\n=== 测试手部动作 ===")
        
        # 举手动作
        print("双手举起...")
        self.send_command("HANDS_UP 1000 0")
        time.sleep(2)
        
        print("放下双手...")
        self.send_command("HANDS_DOWN 1000 0")
        time.sleep(2)
        
        # 单手举起
        print("左手举起...")
        self.send_command("HANDS_UP 1000 1")
        time.sleep(2)
        
        print("右手举起...")
        self.send_command("HANDS_UP 1000 -1")
        time.sleep(2)
        
        # 挥手动作
        print("左手挥手...")
        self.send_command("HAND_WAVE 1000 1")
        time.sleep(3)
        
        print("右手挥手...")
        self.send_command("HAND_WAVE 1000 -1")
        time.sleep(3)
        
        # 双手挥手
        print("双手挥手...")
        self.send_command("HAND_WAVE 1000 0")
        time.sleep(3)
    
    def test_combined_actions(self):
        """测试组合动作"""
        print("\n=== 测试组合动作 ===")
        
        # 举手 + 行走
        print("举手行走...")
        self.send_command("HANDS_UP 1000 0")
        time.sleep(1)
        self.send_command("WALK 2 1000 1 0")
        time.sleep(3)
        self.send_command("HANDS_DOWN 1000 0")
        time.sleep(1)
        
        # 挥手 + 转弯
        print("挥手转弯...")
        self.send_command("HAND_WAVE 500 1")
        time.sleep(2)
        self.send_command("TURN 1 2000 1 0")
        time.sleep(2)
    
    def test_servo_trims(self):
        """测试舵机微调"""
        print("\n=== 测试舵机微调 ===")
        
        # 设置微调值（包括手臂舵机）
        print("设置舵机微调...")
        self.send_command("SET_TRIMS 0 0 0 0 5 -5")  # 左手+5度，右手-5度
        time.sleep(0.5)
        
        # 测试微调效果
        print("测试微调效果...")
        self.send_command("HANDS_UP 1000 0")
        time.sleep(2)
        self.send_command("HANDS_DOWN 1000 0")
        time.sleep(2)
        
        # 重置微调
        print("重置微调...")
        self.send_command("SET_TRIMS 0 0 0 0 0 0")
        time.sleep(0.5)
    
    def run_full_test(self):
        """运行完整测试"""
        if not self.connect():
            return False
        
        try:
            print("开始Otto手臂舵机测试...")
            
            self.test_basic_functions()
            self.test_hand_servos()
            self.test_hand_actions()
            self.test_combined_actions()
            self.test_servo_trims()
            
            print("\n=== 测试完成 ===")
            print("返回初始位置...")
            self.send_command("HOME 1")
            
        except KeyboardInterrupt:
            print("\n测试被用户中断")
        except Exception as e:
            print(f"\n测试出错: {e}")
        finally:
            if self.ser:
                self.ser.close()
                print("连接已关闭")
    
    def interactive_mode(self):
        """交互模式"""
        if not self.connect():
            return
        
        print("\n进入交互模式，输入命令或 'quit' 退出")
        print("示例命令:")
        print("  INIT")
        print("  SERVO_MOVE 4 90")
        print("  HANDS_UP 1000 0")
        print("  HAND_WAVE 1000 1")
        print("  GET_STATUS")
        
        try:
            while True:
                command = input("Otto> ").strip()
                if command.lower() in ['quit', 'exit', 'q']:
                    break
                if command:
                    self.send_command(command)
        except KeyboardInterrupt:
            print("\n退出交互模式")
        finally:
            if self.ser:
                self.ser.close()
                print("连接已关闭")

def main():
    # 检查命令行参数
    port = 'COM3'  # 默认端口，根据实际情况修改
    
    if len(sys.argv) > 1:
        if sys.argv[1] == '--interactive' or sys.argv[1] == '-i':
            # 交互模式
            tester = OttoSerialTester(port)
            tester.interactive_mode()
            return
        else:
            port = sys.argv[1]
    
    # 自动测试模式
    tester = OttoSerialTester(port)
    tester.run_full_test()

if __name__ == "__main__":
    main()
