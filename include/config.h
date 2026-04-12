#ifndef CONFIG_H
#define CONFIG_H

// ========== 硬件引脚定义 ==========

// OLED显示屏 (SPI)
#define OLED_MOSI 11
#define OLED_CLK 12
#define OLED_DC 13
#define OLED_CS 14
#define OLED_RESET 10
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C总线
#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_SPEED 50000

// LED灯带
#define LED_PIN 2
#define NUM_LEDS 30

// GSR传感器 (模拟输入)
#define GSR_PIN 4

// 运动传感器 (I2C)
#define MOTION_SENSOR_ADDR 0x68

// 心率传感器 (I2C)
#define HEART_SENSOR_ADDR 0x57

// ========== 系统配置 ==========
#define SERIAL_BAUD 115200

// ========== 学习配置 ==========
#define MAX_CARDS 10 // 最多支持的卡片数

// ========== 认知负载阈值 ==========
#define LOAD_LOW_THRESHOLD 0.3  // 低负载阈值
#define LOAD_HIGH_THRESHOLD 0.7 // 高负载阈值

#endif