/**
 * @file spi.c
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

#include <string.h>
#include "misaka_device/spi.h"

/**
 * @brief 内部操作函数
 * @param ops spi设备
 * @param message 消息对象
 * @return 发送的字节数
 */
static uint32_t misaka_spi_xfer(misaka_spi_t *ops, misaka_spi_message_t *message)
{
	uint8_t state;
	uint32_t message_length, already_send_length;
	uint32_t send_length;
	uint8_t *recv_buf;
	uint8_t *send_buf;

	misaka_spi_assert(ops != NULL);
	misaka_spi_assert(ops->bus != NULL);
	misaka_spi_assert(ops->bus->send_recv != NULL);
	misaka_spi_assert(ops->bus->send != NULL);
	misaka_spi_assert(ops->bus->recv != NULL);
	misaka_spi_assert(ops->set_sda != NULL);
	misaka_spi_assert(message != NULL);

	if (message->cs_take)
	{
		ops->set_cs(0);
	}

	message_length = message->length;
	recv_buf = message->recv_buf;
	send_buf = message->send_buf;
	while (message_length)
	{
		if (message_length > 0xFFFFFFFF)
		{
			send_length = 0xFFFFFFFF;
			message_length = message_length - 0xFFFFFFFF;
		}
		else
		{
			send_length = message_length;
			message_length = 0;
		}

		already_send_length = message->length - send_length - message_length;
		send_buf = (uint8_t *) message->send_buf + already_send_length;
		recv_buf = (uint8_t *) message->recv_buf + already_send_length;

		if (message->send_buf && message->recv_buf)
		{
			state = ops->bus->send_recv(send_buf, recv_buf, send_length);
		}
		else if (message->send_buf)
		{
			state = ops->bus->send(send_buf, send_length);
		}
		else
		{
			memset((uint8_t *) recv_buf, 0xff, send_length);
			state = ops->bus->recv(recv_buf, send_length);
		}

		if (state != 0)
		{
			message->length = 0;
		}
	}

	if (message->cs_release)
	{
		ops->set_cs(1);
	}

	return message->length;
}

/**
 * @brief 发送数据后发送数据
 * @param ops spi设备
 * @param txbuf1 待发送数据1
 * @param txlen1 发送数据长度1
 * @param txbuf2 待发送数据2
 * @param txlen2 发送数据长度2
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send_then_send(misaka_spi_t *ops, uint8_t *txbuf1, uint32_t txlen1, uint8_t *txbuf2, uint32_t txlen2)
{
	uint32_t result;
	misaka_spi_message_t message;

	misaka_spi_assert(ops != NULL);
	misaka_spi_assert(ops->bus != NULL);

	/** < 发送数据1 */
	message.send_buf = txbuf1;
	message.recv_buf = NULL;
	message.length = txlen1;
	message.cs_take = 1;
	message.cs_release = 0;
	message.next = NULL;

	ops->bus->mutex_take();
	result = misaka_spi_xfer(ops, &message);
	if (result == 0)
	{
		goto __exit;
	}

	/** < 发送数据1 */
	message.send_buf = txbuf2;
	message.recv_buf = NULL;
	message.length = txlen2;
	message.cs_take = 0;
	message.cs_release = 1;
	message.next = NULL;

	result = misaka_spi_xfer(ops, &message);

	if (result == 0)
	{
		goto __exit;
	}

	__exit:
	ops->bus->mutex_release();

	return result ? 1 : 0;
}

/**
 * @brief 发送数据后接收数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param txlen 发送数据长度
 * @param rxbuf 待接收数据
 * @param rxlen 接收数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send_then_recv(misaka_spi_t *ops, uint8_t *txbuf, uint32_t txlen, uint8_t *rxbuf, uint32_t rxlen)
{
	uint32_t result;
	misaka_spi_message_t message;

	misaka_spi_assert(ops != RT_NULL);
	misaka_spi_assert(ops->bus != RT_NULL);

	/** < 发送数据 */
	message.send_buf = txbuf;
	message.recv_buf = NULL;
	message.length = txlen;
	message.cs_take = 1;
	message.cs_release = 0;
	message.next = NULL;

	ops->bus->mutex_take();
	result = misaka_spi_xfer(ops, &message);
	if (result == 0)
	{
		goto __exit;
	}

	/** < 接收数据 */
	message.send_buf = NULL;
	message.recv_buf = rxbuf;
	message.length = rxlen;
	message.cs_take = 0;
	message.cs_release = 1;
	message.next = NULL;

	result = misaka_spi_xfer(ops, &message);
	if (result == 0)
	{
		goto __exit;
	}

	__exit:
	ops->bus->mutex_release();

	return result ? 1 : 0;
}

/**
 * @brief 传输数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param rxbuf 待接收数据
 * @param length 数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_transfer(misaka_spi_t *ops, uint8_t *txbuf, uint8_t *rxbuf, uint32_t length)
{
	uint32_t result;
	misaka_spi_message_t message;

	misaka_spi_assert(ops != RT_NULL);
	misaka_spi_assert(ops->bus != RT_NULL);

	message.send_buf = txbuf;
	message.recv_buf = rxbuf;
	message.length = length;
	message.cs_take = 1;
	message.cs_release = 1;
	message.next = NULL;

	ops->bus->mutex_take();
	result = misaka_spi_xfer(ops, &message);
	if (result == 0)
	{
		goto __exit;
	}

	__exit:
	ops->bus->mutex_release();

	return result ? 1 : 0;
}

/**
 * @brief 自定义传输数据
 * @param ops spi设备
 * @param message 消息
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_transfer_message(misaka_spi_t *ops, misaka_spi_message_t *message)
{
	uint32_t result;
	misaka_spi_message_t *index;

	misaka_spi_assert(ops != NULL);

	index = message;
	if (index == NULL)
	{
		return 1;
	}

	ops->bus->mutex_take();
	while (index != NULL)
	{
		result = misaka_spi_xfer(ops, index);
		if (result == 0)
		{
			goto __exit;
		}

		index = index->next;
	}

	__exit:
	ops->bus->mutex_release();

	return result ? 1 : 0;
}

/**
 * @brief 发送一次数据
 * @param ops spi设备
 * @param txbuf 待发送数据
 * @param length 数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_send(misaka_spi_t *ops, uint8_t *txbuf, uint32_t length)
{
	return misaka_spi_transfer(ops, txbuf, NULL, length);
}

/**
 * @brief 接收一次数据
 * @param ops spi设备
 * @param rxbuf 待接收数据
 * @param length 数据长度
 * @return 0:成功 1:失败
 */
uint8_t misaka_spi_recv(misaka_spi_t *ops, uint8_t *rxbuf, uint32_t length)
{
	return misaka_spi_transfer(ops, NULL, rxbuf, length);
}

