#include "misaka_device_soft_i2c.h"

static misaka_device_soft_i2c i2c_obj;

/**
 * @brief 设置sda引脚电平
 * @param  level            0: 低电平
 *                          1: 高电平
 */
static void set_sda(uint8_t level)
{

}

/**
 * @brief 设置scl引脚电平
 * @param  level            0: 低电平
 *                          1: 高电平
 */
static void set_scl(uint8_t level)
{

}

/**
 * @brief 读取sda引脚电平
 * @return 0 @c 低电平
 * @return 1 @c 高电平
 */
static uint8_t get_sda()
{

}

/**
 * @brief 获取互斥量，如果为裸机系统，空函数即可
 */
static void mutex_take()
{

}

/**
 * @brief 释放互斥量，如果为裸机系统，空函数即可
 */
static void mutex_release()
{

}

/**
 * @brief 延时us
 * @param  ms               desc
 */
static void delay_us(uint16_t ms)
{

}

/**
 * @brief 设置sda引脚为输出模式（硬件无上拉时需要添加）
 */
static void set_sda_out()
{

}

/**
 * @brief 设置sda引脚为输入模式（硬件无上拉时需要添加）
 */
static void set_sda_in()
{

}

/**
 * @brief I2C读写错误回调函数
 * @param  ops              i2c设备
 */
void misaka_device_soft_i2c_error_callback(misaka_device_soft_i2c_t ops)
{

}

misaka_device_soft_i2c_t misaka_device_soft_i2c_port_init()
{
    i2c_obj.delay_us = delay_us;
    i2c_obj.get_sda = get_sda;
    i2c_obj.mutex_release = mutex_release;
    i2c_obj.mutex_take = mutex_take;
    i2c_obj.set_scl = set_scl;
    i2c_obj.set_sda = set_sda;
    i2c_obj.set_sda_out = set_sda_out;
    i2c_obj.set_sda_in = set_sda_in;
    i2c_obj.us = 1;

    set_sda(1);
    set_scl(1);

    misaka_device_soft_i2c_init(&i2c_obj);

    return &i2c_obj;
}

