/*
 * ESP32 MCP集成示例 - Otto串口控制
 * 
 * 这个文件展示了如何在xiaozhi-esp32系统中集成Arduino Nano的Otto串口控制
 * 可以参考sparkbot的实现方式，通过串口与Arduino Nano通信
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/uart.h>
#include <cJSON.h>
#include "mcp_server.h"

#define TAG "OttoSerialMCP"

// UART配置
#define OTTO_UART_NUM           UART_NUM_1
#define OTTO_UART_TXD_PIN       17  // 连接到Arduino Nano的RX
#define OTTO_UART_RXD_PIN       16  // 连接到Arduino Nano的TX
#define OTTO_UART_BAUDRATE      115200
#define OTTO_UART_BUFFER_SIZE   1024

class OttoSerialMCP {
private:
    QueueHandle_t uart_queue_;
    TaskHandle_t uart_task_handle_;
    bool is_initialized_;
    
    struct OttoCommand {
        char command[128];
        char response[128];
        bool completed;
        TickType_t timestamp;
    };
    
    QueueHandle_t command_queue_;
    
public:
    OttoSerialMCP() : is_initialized_(false) {
        command_queue_ = xQueueCreate(10, sizeof(OttoCommand));
    }
    
    bool Initialize() {
        if (is_initialized_) {
            return true;
        }
        
        // 配置UART
        uart_config_t uart_config = {
            .baud_rate = OTTO_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        
        ESP_ERROR_CHECK(uart_driver_install(OTTO_UART_NUM, OTTO_UART_BUFFER_SIZE, 
                                          OTTO_UART_BUFFER_SIZE, 10, &uart_queue_, 0));
        ESP_ERROR_CHECK(uart_param_config(OTTO_UART_NUM, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(OTTO_UART_NUM, OTTO_UART_TXD_PIN, 
                                   OTTO_UART_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        
        // 创建UART处理任务
        xTaskCreate(UartTaskWrapper, "otto_uart_task", 4096, this, 5, &uart_task_handle_);
        
        is_initialized_ = true;
        ESP_LOGI(TAG, "Otto串口MCP已初始化");
        
        return true;
    }
    
    static void UartTaskWrapper(void* arg) {
        static_cast<OttoSerialMCP*>(arg)->UartTask();
    }
    
    void UartTask() {
        uart_event_t event;
        uint8_t* data = (uint8_t*) malloc(OTTO_UART_BUFFER_SIZE);
        
        while (true) {
            if (xQueueReceive(uart_queue_, (void*)&event, pdMS_TO_TICKS(100))) {
                switch (event.type) {
                    case UART_DATA:
                        if (event.size > 0) {
                            int len = uart_read_bytes(OTTO_UART_NUM, data, event.size, pdMS_TO_TICKS(100));
                            if (len > 0) {
                                data[len] = '\\0';
                                HandleResponse((char*)data);
                            }
                        }
                        break;
                    case UART_FIFO_OVF:
                        ESP_LOGW(TAG, "UART FIFO溢出");
                        uart_flush_input(OTTO_UART_NUM);
                        xQueueReset(uart_queue_);
                        break;
                    case UART_BUFFER_FULL:
                        ESP_LOGW(TAG, "UART缓冲区满");
                        uart_flush_input(OTTO_UART_NUM);
                        xQueueReset(uart_queue_);
                        break;
                    default:
                        break;
                }
            }
        }
        
        free(data);
    }
    
    void HandleResponse(const char* response) {
        ESP_LOGI(TAG, "收到Arduino响应: %s", response);
        // 这里可以处理Arduino的响应，更新状态等
    }
    
    bool SendCommand(const char* command, char* response = nullptr, int timeout_ms = 2000) {
        if (!is_initialized_) {
            ESP_LOGE(TAG, "串口未初始化");
            return false;
        }
        
        // 发送命令
        char cmd_with_newline[256];
        snprintf(cmd_with_newline, sizeof(cmd_with_newline), "%s\\n", command);
        
        int len = uart_write_bytes(OTTO_UART_NUM, cmd_with_newline, strlen(cmd_with_newline));
        if (len <= 0) {
            ESP_LOGE(TAG, "发送命令失败: %s", command);
            return false;
        }
        
        ESP_LOGI(TAG, "发送命令: %s", command);
        
        // 如果需要响应，等待响应
        if (response != nullptr) {
            // 简化实现 - 实际应用中需要更复杂的响应匹配
            vTaskDelay(pdMS_TO_TICKS(timeout_ms));
            strcpy(response, "OK"); // 简化响应
        }
        
        return true;
    }
    
    // MCP工具注册
    void RegisterMcpTools() {
        auto& mcp_server = McpServer::GetInstance();
        
        ESP_LOGI(TAG, "注册Otto MCP工具...");
        
        // 基础移动工具
        mcp_server.AddTool("otto.walk_forward",
            "让Otto机器人向前行走",
            [this](const cJSON* args) -> cJSON* {
                return HandleWalkForward(args);
            });
            
        mcp_server.AddTool("otto.walk_backward", 
            "让Otto机器人向后行走",
            [this](const cJSON* args) -> cJSON* {
                return HandleWalkBackward(args);
            });
            
        mcp_server.AddTool("otto.turn_left",
            "让Otto机器人向左转",
            [this](const cJSON* args) -> cJSON* {
                return HandleTurnLeft(args);
            });
            
        mcp_server.AddTool("otto.turn_right",
            "让Otto机器人向右转", 
            [this](const cJSON* args) -> cJSON* {
                return HandleTurnRight(args);
            });
            
        mcp_server.AddTool("otto.jump",
            "让Otto机器人跳跃",
            [this](const cJSON* args) -> cJSON* {
                return HandleJump(args);
            });
            
        // 特殊动作工具
        mcp_server.AddTool("otto.swing",
            "让Otto机器人摇摆",
            [this](const cJSON* args) -> cJSON* {
                return HandleSwing(args);
            });
            
        mcp_server.AddTool("otto.moonwalk",
            "让Otto机器人做太空步",
            [this](const cJSON* args) -> cJSON* {
                return HandleMoonwalk(args);
            });
            
        // 手部动作工具
        mcp_server.AddTool("otto.hands_up",
            "让Otto机器人举手",
            [this](const cJSON* args) -> cJSON* {
                return HandleHandsUp(args);
            });
            
        mcp_server.AddTool("otto.hand_wave",
            "让Otto机器人挥手",
            [this](const cJSON* args) -> cJSON* {
                return HandleHandWave(args);
            });
            
        // 系统控制工具
        mcp_server.AddTool("otto.home",
            "让Otto机器人回到初始位置",
            [this](const cJSON* args) -> cJSON* {
                return HandleHome(args);
            });
            
        mcp_server.AddTool("otto.stop",
            "停止Otto机器人的当前动作",
            [this](const cJSON* args) -> cJSON* {
                return HandleStop(args);
            });
            
        mcp_server.AddTool("otto.get_status",
            "获取Otto机器人的状态",
            [this](const cJSON* args) -> cJSON* {
                return HandleGetStatus(args);
            });
    }
    
private:
    // MCP工具处理函数
    cJSON* HandleWalkForward(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 2);
        int speed = GetIntParam(args, "speed", 1000);
        int amount = GetIntParam(args, "amount", 30);
        
        char command[128];
        snprintf(command, sizeof(command), "WALK:%d,%d,1,%d", steps, speed, amount);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始向前行走");
        } else {
            return CreateErrorResponse("发送行走命令失败");
        }
    }
    
    cJSON* HandleWalkBackward(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 2);
        int speed = GetIntParam(args, "speed", 1000);
        int amount = GetIntParam(args, "amount", 30);
        
        char command[128];
        snprintf(command, sizeof(command), "WALK:%d,%d,-1,%d", steps, speed, amount);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始向后行走");
        } else {
            return CreateErrorResponse("发送后退命令失败");
        }
    }
    
    cJSON* HandleTurnLeft(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 1);
        int speed = GetIntParam(args, "speed", 2000);
        
        char command[128];
        snprintf(command, sizeof(command), "TURN:%d,%d,1,0", steps, speed);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始左转");
        } else {
            return CreateErrorResponse("发送左转命令失败");
        }
    }
    
    cJSON* HandleTurnRight(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 1);
        int speed = GetIntParam(args, "speed", 2000);
        
        char command[128];
        snprintf(command, sizeof(command), "TURN:%d,%d,-1,0", steps, speed);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始右转");
        } else {
            return CreateErrorResponse("发送右转命令失败");
        }
    }
    
    cJSON* HandleJump(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 1);
        int speed = GetIntParam(args, "speed", 2000);
        
        char command[128];
        snprintf(command, sizeof(command), "JUMP:%d,%d", steps, speed);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始跳跃");
        } else {
            return CreateErrorResponse("发送跳跃命令失败");
        }
    }
    
    cJSON* HandleSwing(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 2);
        int speed = GetIntParam(args, "speed", 1000);
        int height = GetIntParam(args, "height", 20);
        
        char command[128];
        snprintf(command, sizeof(command), "SWING:%d,%d,%d", steps, speed, height);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始摇摆");
        } else {
            return CreateErrorResponse("发送摇摆命令失败");
        }
    }
    
    cJSON* HandleMoonwalk(const cJSON* args) {
        int steps = GetIntParam(args, "steps", 2);
        int speed = GetIntParam(args, "speed", 900);
        int height = GetIntParam(args, "height", 20);
        int direction = GetIntParam(args, "direction", 1);
        
        char command[128];
        snprintf(command, sizeof(command), "MOONWALK:%d,%d,%d,%d", steps, speed, height, direction);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始太空步");
        } else {
            return CreateErrorResponse("发送太空步命令失败");
        }
    }
    
    cJSON* HandleHandsUp(const cJSON* args) {
        int speed = GetIntParam(args, "speed", 1000);
        int direction = GetIntParam(args, "direction", 0); // 0=双手
        
        char command[128];
        snprintf(command, sizeof(command), "HANDS_UP:%d,%d", speed, direction);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto举起手臂");
        } else {
            return CreateErrorResponse("发送举手命令失败");
        }
    }
    
    cJSON* HandleHandWave(const cJSON* args) {
        int speed = GetIntParam(args, "speed", 1000);
        int direction = GetIntParam(args, "direction", 1); // 1=左手
        
        char command[128];
        snprintf(command, sizeof(command), "HAND_WAVE:%d,%d", speed, direction);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto开始挥手");
        } else {
            return CreateErrorResponse("发送挥手命令失败");
        }
    }
    
    cJSON* HandleHome(const cJSON* args) {
        bool hands_down = GetBoolParam(args, "hands_down", true);
        
        char command[128];
        snprintf(command, sizeof(command), "HOME:%d", hands_down ? 1 : 0);
        
        if (SendCommand(command)) {
            return CreateSuccessResponse("Otto回到初始位置");
        } else {
            return CreateErrorResponse("发送复位命令失败");
        }
    }
    
    cJSON* HandleStop(const cJSON* args) {
        if (SendCommand("STOP")) {
            return CreateSuccessResponse("Otto停止运动");
        } else {
            return CreateErrorResponse("发送停止命令失败");
        }
    }
    
    cJSON* HandleGetStatus(const cJSON* args) {
        char response[256];
        if (SendCommand("GET_STATUS", response, 1000)) {
            cJSON* result = cJSON_CreateObject();
            cJSON_AddStringToObject(result, "status", "success");
            cJSON_AddStringToObject(result, "robot_status", response);
            return result;
        } else {
            return CreateErrorResponse("获取状态失败");
        }
    }
    
    // 辅助函数
    int GetIntParam(const cJSON* args, const char* name, int default_value) {
        const cJSON* param = cJSON_GetObjectItem(args, name);
        if (cJSON_IsNumber(param)) {
            return param->valueint;
        }
        return default_value;
    }
    
    bool GetBoolParam(const cJSON* args, const char* name, bool default_value) {
        const cJSON* param = cJSON_GetObjectItem(args, name);
        if (cJSON_IsBool(param)) {
            return cJSON_IsTrue(param);
        }
        return default_value;
    }
    
    cJSON* CreateSuccessResponse(const char* message) {
        cJSON* response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "success");
        cJSON_AddStringToObject(response, "message", message);
        return response;
    }
    
    cJSON* CreateErrorResponse(const char* error) {
        cJSON* response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "error", error);
        return response;
    }
};

// 全局实例
static OttoSerialMCP* g_otto_serial_mcp = nullptr;

// 初始化函数
extern "C" void InitializeOttoSerialMCP() {
    if (g_otto_serial_mcp == nullptr) {
        g_otto_serial_mcp = new OttoSerialMCP();
        if (g_otto_serial_mcp->Initialize()) {
            g_otto_serial_mcp->RegisterMcpTools();
            ESP_LOGI(TAG, "Otto串口MCP系统已初始化");
        } else {
            ESP_LOGE(TAG, "Otto串口MCP初始化失败");
            delete g_otto_serial_mcp;
            g_otto_serial_mcp = nullptr;
        }
    }
}

// 在board.cc中调用此函数进行初始化
extern "C" void board_init_otto_serial() {
    InitializeOttoSerialMCP();
}
