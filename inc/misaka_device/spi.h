/**
 * @file spi.h
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

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include <stddef.h>

#define misaka_spi_assert(expr)  ((void)0U)

struct misaka_spi_bus_struct
{
	uint8_t (*send_recv)(uint8_t *txbuf, uint8_t *rxbuf, uint32_t length);/**< 发送的时候接收数据 */
	uint8_t (*send)(uint8_t *txbuf, uint32_t length);/**< 发送数据 */
	uint8_t (*recv)(uint8_t *rxbuf, uint32_t length);/**< 接收数据 */
	void (*mutex_take)();/**< 获取互斥量，如果为裸机系统，空函数即可 */
	void (*mutex_release)();/**< 释放互斥量，如果为裸机系统，空函数即可 */
};

typedef struct misaka_spi_bus_struct misaka_spi_bus_t;

struct misaka_spi_message_struct
{
	uint8_t *send_buf;
	uint8_t *recv_buf;
	uint32_t length;
	struct misaka_spi_message_struct *next;

	unsigned cs_take: 1;
	unsigned cs_release: 1;
};
typedef struct misaka_spi_message_struct misaka_spi_message_t;

struct misaka_spi_struct
{
	void (*set_cs)(uint8_t state);                        /**< 设置cs引脚电平 */
	misaka_spi_bus_t *bus;
};

typedef struct misaka_spi_struct misaka_spi_t;

/**
 * @brief 发送数据后发送数据
 * @param ops spi设备
 * @param txbuf1 待发送数据1
 * @param txlen1 发送数据长度1
 * @param txbuf2 待发送数据2
 * @param txlen2 发送数据长度2
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send_then_send(misaka_spi_t *ops, uint8_t *txbuf1, uint32_t txlen1, uint8_t *txbuf2, uint32_t txlen2);

/**
 * @brief 发送数据后接收数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param txlen 发送数据长度
 * @param rxbuf 待接收数据
 * @param rxlen 接收数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send_then_recv(misaka_spi_t *ops, uint8_t *txbuf, uint32_t txlen, uint8_t *rxbuf, uint32_t rxlen);

/**
 * @brief 传输数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param rxbuf 待接收数据
 * @param length 数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_transfer(misaka_spi_t *ops, uint8_t *txbuf, uint8_t *rxbuf, uint32_t length);

/**
 * @brief 自定义传输数据
 * @param ops spi设备
 * @param message 消息
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_transfer_message(misaka_spi_t *ops, misaka_spi_message_t *message);

/**
 * @brief 发送一次数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param length 数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send(misaka_spi_t *ops, uint8_t *txbuf, uint32_t length);

#endif //__SPI_H__
