/**
 * @file spi_port.c
 * @brief
 * @author xqyjlj (xqyjlj@126.com)
 * @version 0.0
 * @date 2021-10-23
 * @copyright Copyright © 2021-2021 xqyjlj<xqyjlj@126.com>
 * @SPDX-License-Identifier: Apache-2.0
 *
 * ********************************************************************************
 * @par ChangeLog:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2021-10-23 <td>0.0     <td>xqyjlj  <td>内容
 * </table>
 * ********************************************************************************
 */

#include "misaka_device/spi.h"

static misaka_spi_bus_t s_misaka_spi1_bus_obj;
misaka_spi_bus_t *misaka_spi1_bus_obj = NULL;
static misaka_spi_t s_misaka_spi11_obj;
misaka_spi_t *misaka_spi11_obj = NULL;

/**
 * @brief 发送接收数据
 * @param txbuf 待发送数据
 * @param rxbuf 待接收数据
 * @param length 待数据长度
 * @return  0:成功 1:失败
 */
static uint8_t send_recv(uint8_t *txbuf, uint8_t *rxbuf, uint32_t length)
{

}

/**
 * @brief 接收消息
 * @param txbuf 待发送数据
 * @param length 待发送数据长度
 * @return  0:成功 1:失败
 */
static uint8_t send(uint8_t *txbuf, uint32_t length)
{

}

/**
 * @brief 接收消息
 * @param rxbuf 待接收数据
 * @param length 待接收数据长度
 * @return  0:成功 1:失败
 */
static uint8_t recv(uint8_t *rxbuf, uint32_t length)
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
 * @brief 设置cs引脚电平
 * @param level 0: 低电平 1: 高电平
 */
static void set_cs(uint8_t state)
{

}

static int misaka_spi_port_init()
{

	s_misaka_spi1_bus_obj.mutex_release = mutex_release;
	s_misaka_spi1_bus_obj.mutex_take = mutex_take;
	s_misaka_spi1_bus_obj.recv = recv;
	s_misaka_spi1_bus_obj.send = send;
	s_misaka_spi1_bus_obj.send_recv = send_recv;

	misaka_spi1_bus_obj = &s_misaka_spi1_bus_obj;

	s_misaka_spi11_obj.set_cs = set_cs;
	s_misaka_spi11_obj.bus = misaka_spi1_bus_obj;
	misaka_spi11_obj = &s_misaka_spi11_obj;

	return 1;
}
