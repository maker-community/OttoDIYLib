#!/usr/bin/env python3
"""
Otto Serial Control Test Script
用于测试Arduino Nano版本的Otto机器人串口控制

使用方法:
1. 将Arduino连接到电脑
2. 修改下面的COM_PORT为正确的串口
3. 运行脚本: python otto_serial_test.py

命令格式示例:
- INIT
- HOME:1
- WALK:2,1000,1,30
- TURN:1,2000,-1,0
- JUMP:1,2000
- HANDS_UP:1000,0
- HAND_WAVE:1000,1
"""

import serial
import time
import sys

# 配置串口参数
COM_PORT = 'COM3'  # 根据实际情况修改
BAUDRATE = 115200
TIMEOUT = 2

class OttoSerialController:
    def __init__(self, port, baudrate=115200, timeout=2):
        try:
            self.serial = serial.Serial(port, baudrate, timeout=timeout)
            time.sleep(2)  # 等待Arduino重启
            print(f"连接到Otto机器人: {port}")
        except Exception as e:
            print(f"无法连接到串口 {port}: {e}")
            sys.exit(1)
    
    def send_command(self, command):
        """发送命令并获取响应"""
        try:
            # 发送命令
            cmd_bytes = (command + '\n').encode('utf-8')
            self.serial.write(cmd_bytes)
            print(f"发送: {command}")
            
            # 读取响应
            response = self.serial.readline().decode('utf-8').strip()
            print(f"响应: {response}")
            return response
        except Exception as e:
            print(f"通信错误: {e}")
            return None
    
    def init_robot(self):
        """初始化机器人"""
        return self.send_command("INIT")
    
    def home(self, hands_down=True):
        """回到初始位置"""
        return self.send_command(f"HOME:{1 if hands_down else 0}")
    
    def walk(self, steps=2, speed=1000, direction=1, amount=30):
        """行走"""
        return self.send_command(f"WALK:{steps},{speed},{direction},{amount}")
    
    def turn(self, steps=1, speed=2000, direction=1, amount=0):
        """转向"""
        return self.send_command(f"TURN:{steps},{speed},{direction},{amount}")
    
    def jump(self, steps=1, speed=2000):
        """跳跃"""
        return self.send_command(f"JUMP:{steps},{speed}")
    
    def swing(self, steps=1, speed=1000, height=20):
        """摇摆"""
        return self.send_command(f"SWING:{steps},{speed},{height}")
    
    def moonwalk(self, steps=1, speed=900, height=20, direction=1):
        """太空步"""
        return self.send_command(f"MOONWALK:{steps},{speed},{height},{direction}")
    
    def bend(self, steps=1, speed=1400, direction=1):
        """弯曲"""
        return self.send_command(f"BEND:{steps},{speed},{direction}")
    
    def shake_leg(self, steps=1, speed=2000, direction=1):
        """摇腿"""
        return self.send_command(f"SHAKE_LEG:{steps},{speed},{direction}")
    
    def up_down(self, steps=1, speed=1000, height=20):
        """上下运动"""
        return self.send_command(f"UPDOWN:{steps},{speed},{height}")
    
    def tiptoe_swing(self, steps=1, speed=900, height=20):
        """踮脚摇摆"""
        return self.send_command(f"TIPTOE_SWING:{steps},{speed},{height}")
    
    def jitter(self, steps=1, speed=500, height=20):
        """抖动"""
        return self.send_command(f"JITTER:{steps},{speed},{height}")
    
    def ascending_turn(self, steps=1, speed=900, height=20):
        """上升转向"""
        return self.send_command(f"ASCENDING_TURN:{steps},{speed},{height}")
    
    def crusaito(self, steps=1, speed=900, height=20, direction=1):
        """十字步"""
        return self.send_command(f"CRUSAITO:{steps},{speed},{height},{direction}")
    
    def flapping(self, steps=1, speed=1000, height=20, direction=1):
        """拍打动作"""
        return self.send_command(f"FLAPPING:{steps},{speed},{height},{direction}")
    
    def hands_up(self, speed=1000, direction=0):
        """举手"""
        return self.send_command(f"HANDS_UP:{speed},{direction}")
    
    def hands_down(self, speed=1000, direction=0):
        """放手"""
        return self.send_command(f"HANDS_DOWN:{speed},{direction}")
    
    def hand_wave(self, speed=1000, direction=1):
        """挥手"""
        return self.send_command(f"HAND_WAVE:{speed},{direction}")
    
    def set_trims(self, yl=0, yr=0, rl=0, rr=0, lh=0, rh=0):
        """设置舵机微调"""
        return self.send_command(f"SET_TRIMS:{yl},{yr},{rl},{rr},{lh},{rh}")
    
    def enable_servo_limit(self, speed_limit=240):
        """启用舵机速度限制"""
        return self.send_command(f"ENABLE_LIMIT:{speed_limit}")
    
    def disable_servo_limit(self):
        """禁用舵机速度限制"""
        return self.send_command("DISABLE_LIMIT")
    
    def get_status(self):
        """获取状态"""
        return self.send_command("GET_STATUS")
    
    def move_servo(self, servo, position):
        """单独控制舵机"""
        return self.send_command(f"SERVO_MOVE:{servo},{position}")
    
    def stop(self):
        """停止运动"""
        return self.send_command("STOP")
    
    def close(self):
        """关闭串口连接"""
        if self.serial:
            self.serial.close()
            print("串口连接已关闭")

def demo_basic_movements(otto):
    """基础动作演示"""
    print("\n=== 基础动作演示 ===")
    
    print("初始化机器人...")
    otto.init_robot()
    time.sleep(1)
    
    print("回到初始位置...")
    otto.home()
    time.sleep(2)
    
    print("前进2步...")
    otto.walk(steps=2, speed=1000, direction=1, amount=30)
    time.sleep(3)
    
    print("左转1步...")
    otto.turn(steps=1, speed=2000, direction=1)
    time.sleep(3)
    
    print("跳跃...")
    otto.jump(steps=1, speed=2000)
    time.sleep(2)
    
    print("回到初始位置...")
    otto.home()
    time.sleep(2)

def demo_hand_movements(otto):
    """手部动作演示"""
    print("\n=== 手部动作演示 ===")
    
    print("双手举起...")
    otto.hands_up(speed=1000, direction=0)
    time.sleep(2)
    
    print("双手放下...")
    otto.hands_down(speed=1000, direction=0)
    time.sleep(2)
    
    print("左手挥手...")
    otto.hand_wave(speed=1000, direction=1)
    time.sleep(3)
    
    print("右手挥手...")
    otto.hand_wave(speed=1000, direction=-1)
    time.sleep(3)
    
    print("双手挥手...")
    otto.hand_wave(speed=1000, direction=0)
    time.sleep(3)

def demo_advanced_movements(otto):
    """高级动作演示"""
    print("\n=== 高级动作演示 ===")
    
    print("摇摆...")
    otto.swing(steps=2, speed=1000, height=20)
    time.sleep(3)
    
    print("太空步...")
    otto.moonwalk(steps=2, speed=900, height=20, direction=1)
    time.sleep(4)
    
    print("抖动...")
    otto.jitter(steps=2, speed=500, height=20)
    time.sleep(3)
    
    print("弯曲...")
    otto.bend(steps=1, speed=1400, direction=1)
    time.sleep(2)
    
    print("摇腿...")
    otto.shake_leg(steps=1, speed=2000, direction=1)
    time.sleep(3)

def interactive_mode(otto):
    """交互模式"""
    print("\n=== 交互模式 ===")
    print("输入命令控制Otto机器人 (输入'help'查看帮助, 'quit'退出):")
    
    while True:
        try:
            command = input("Otto> ").strip()
            
            if command.lower() == 'quit':
                break
            elif command.lower() == 'help':
                print_help()
            elif command:
                otto.send_command(command)
                
        except KeyboardInterrupt:
            print("\n退出交互模式...")
            break
        except Exception as e:
            print(f"错误: {e}")

def print_help():
    """打印帮助信息"""
    help_text = """
可用命令:
  INIT                    - 初始化机器人
  HOME:hands_down         - 回到初始位置 (hands_down: 0或1)
  WALK:steps,speed,dir,amount - 行走 (dir: 1=前进, -1=后退)
  TURN:steps,speed,dir,amount - 转向 (dir: 1=左转, -1=右转)
  JUMP:steps,speed        - 跳跃
  SWING:steps,speed,height - 摇摆
  MOONWALK:steps,speed,height,dir - 太空步
  HANDS_UP:speed,dir      - 举手 (dir: 0=双手, 1=左手, -1=右手)
  HANDS_DOWN:speed,dir    - 放手
  HAND_WAVE:speed,dir     - 挥手
  GET_STATUS              - 获取状态
  STOP                    - 停止运动
  
示例:
  WALK:2,1000,1,30       - 前进2步，速度1000，手臂摆动幅度30
  TURN:1,2000,-1,0       - 右转1步，速度2000
  HANDS_UP:1000,0        - 双手举起，速度1000
  HAND_WAVE:1000,1       - 左手挥手，速度1000
"""
    print(help_text)

def main():
    print("Otto Serial Control Test Script")
    print("==============================")
    
    # 创建控制器
    try:
        otto = OttoSerialController(COM_PORT, BAUDRATE, TIMEOUT)
    except Exception as e:
        print(f"初始化失败: {e}")
        return
    
    try:
        # 获取初始状态
        otto.get_status()
        time.sleep(1)
        
        # 运行演示
        while True:
            print("\n选择模式:")
            print("1. 基础动作演示")
            print("2. 手部动作演示") 
            print("3. 高级动作演示")
            print("4. 交互模式")
            print("5. 退出")
            
            choice = input("请选择 (1-5): ").strip()
            
            if choice == '1':
                demo_basic_movements(otto)
            elif choice == '2':
                demo_hand_movements(otto)
            elif choice == '3':
                demo_advanced_movements(otto)
            elif choice == '4':
                interactive_mode(otto)
            elif choice == '5':
                break
            else:
                print("无效选择，请重试")
                
    except KeyboardInterrupt:
        print("\n程序被用户中断")
    except Exception as e:
        print(f"运行错误: {e}")
    finally:
        otto.close()

if __name__ == "__main__":
    main()
