#include "misaka_device_soft_i2c.h"

#define LOG_NAME "misaka_device_soft_i2c"

/**
 * @brief 产生起始信号
 * @param  ops              i2c设备
 */
static void misaka_device_soft_i2c_start(misaka_device_soft_i2c_t ops)
{
    ops->set_sda_out();
    ops->set_sda(0);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_scl(0);
}

/**
 * @brief 重复产生起始信号，一般是连续读写中
 * @param  ops              i2c设备
 */
static void misaka_device_soft_i2c_restart(misaka_device_soft_i2c_t ops)
{
    ops->set_sda_out();
    ops->set_sda(1);
    ops->set_scl(1);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_sda(0);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_scl(0);
}

/**
 * @brief 产生停止信号
 * @param  ops              i2c设备
 */
static void misaka_device_soft_i2c_stop(misaka_device_soft_i2c_t ops)
{
    ops->set_sda_out();

    ops->set_sda(0);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_scl(1);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_sda(1);
    ops->delay_us(ops->us);
}

/**
 * @brief 等待应答信号到来
 * @param  ops              i2c设备
 * @return 0 @c 接收应答失败
 * @return 1 @c 接收应答成功
 */
static uint8_t misaka_device_soft_i2c_wait_ack(misaka_device_soft_i2c_t ops)
{
    uint8_t ack;

    ops->set_sda_in();

    ops->set_sda(1);
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_scl(1);
    ack = !ops->get_sda();
    ops->set_scl(0);

    return ack;
}

/**
 * @brief 发送一个字节
 * @param  ops              i2c设备
 * @param  data             待发送数据
 * @return 0 @c 无应答
 * @return 1 @c 有应答
 */
static uint8_t misaka_device_soft_i2c_write_byte(misaka_device_soft_i2c_t ops, uint8_t data)
{
    int8_t i;
    uint8_t bit;

    ops->set_sda_out();

    for (i = 7; i >= 0; i--)
    {
        ops->set_scl(0);
        bit = (data >> i) & 1;
        ops->set_sda(bit);
        ops->delay_us((ops->us + 1) >> 1);
        ops->set_scl(1);
        ops->delay_us((ops->us + 1) >> 1);
    }
    ops->set_scl(0);
    ops->delay_us((ops->us + 1) >> 1);

    return misaka_device_soft_i2c_wait_ack(ops);
}

/**
 * @brief 读1个字节
 * @param  ops              i2c设备
 * @return uint8_t @c 读取到的1个字节
 */
static uint8_t misaka_device_soft_i2c_read_byte(misaka_device_soft_i2c_t ops)
{
    uint8_t i;
    uint8_t data = 0;

    ops->set_sda_in();

    ops->set_sda(1);
    ops->delay_us((ops->us + 1) >> 1);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        ops->set_scl(1);

        if (ops->get_sda())
        {
            data |= 1;
        }
        ops->set_scl(0);
        ops->delay_us(ops->us);
    }

    return data;
}

/**
 * @brief 发送多字节
 * @param  ops              i2c设备
 * @param  msg              消息对象
 * @return uint32_t @c 发送的字节数
 */
static uint32_t misaka_device_soft_i2c_send_bytes(misaka_device_soft_i2c_t ops, misaka_device_soft_i2c_message_t msg)
{
    uint8_t ret;
    uint32_t bytes = 0;
    uint8_t* ptr = msg->buf;
    uint32_t len = msg->len;
    uint16_t ignore_nack = msg->flags & misaka_device_soft_i2c_IGNORE_NACK;

    while (len > 0)
    {
        ret = misaka_device_soft_i2c_write_byte(ops, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0)))
        {
            len--;
            ptr++;
            bytes++;
        }
        else if (ret == 0)
        {
            return 0;
        }
        else
        {
            return ret;
        }
    }

    return bytes;
}

/**
 * @brief 产生ACK应答
 * @param  ops              i2c设备
 * @param  ack              0: 不产生
 *                          1: 产生
 */
static void misaka_device_soft_i2c_send_ack_or_nack(misaka_device_soft_i2c_t ops, uint8_t ack)
{
    ops->set_sda_out();

    if (ack)
    {
        ops->set_sda(0);
    }
    ops->delay_us((ops->us + 1) >> 1);
    ops->set_scl(1);
    ops->set_scl(0);
}

/**
 * @brief 接收多字节
 * @param  ops              i2c设备
 * @param  msg              消息对象
 * @return uint32_t @c 接收多字节数
 */
static uint32_t misaka_device_soft_i2c_recv_bytes(misaka_device_soft_i2c_t ops, misaka_device_soft_i2c_message_t msg)
{
    uint8_t val;
    uint32_t bytes = 0;   /* actual bytes */
    uint8_t* ptr = msg->buf;
    uint32_t len = msg->len;
    uint16_t flags = msg->flags;

    while (len > 0)
    {
        val = misaka_device_soft_i2c_read_byte(ops);
        if (val >= 0)
        {
            *ptr = val;
            bytes++;
        }
        else
        {
            break;
        }

        ptr++;
        len--;

        if (!(flags & misaka_device_soft_i2c_NO_READ_ACK))
        {
            misaka_device_soft_i2c_send_ack_or_nack(ops, len);
        }
    }

    return bytes;
}

/**
 * @brief 发送地址，启动i2c总线，可重复
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  retries          重复次数
 * @return 0 @c 有应答
 * @return 1 @c 无应答
 */
static uint8_t misaka_device_soft_i2c_send_address(misaka_device_soft_i2c_t ops, uint8_t addr, uint8_t retries)
{
    uint8_t i;
    uint8_t ret = 0;

    for (i = 0; i <= retries; i++)
    {
        ret = misaka_device_soft_i2c_write_byte(ops, addr);
        if (ret == 1 || i == retries)
        {
            break;
        }
        misaka_device_soft_i2c_stop(ops);
        ops->delay_us(ops->us);
        misaka_device_soft_i2c_start(ops);
    }

    return ret;
}

/**
 * @brief 发送i2c从地址
 * @param  ops              i2c设备
 * @param  msg              消息对象
 * @return 0 @c 有应答
 * @return 1 @c 无应答
 */
static uint8_t misaka_device_soft_i2c_bit_send_address(misaka_device_soft_i2c_t ops, misaka_device_soft_i2c_message_t msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & misaka_device_soft_i2c_IGNORE_NACK;

    uint8_t addr1, addr2, retries;
    uint8_t ret;

    retries = ignore_nack ? 0 : 1;

    if (flags & misaka_device_soft_i2c_ADDR_10BIT)
    {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;

        ret = misaka_device_soft_i2c_send_address(ops, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            return 1;
        }

        ret = misaka_device_soft_i2c_write_byte(ops, addr2);
        if ((ret != 1) && !ignore_nack)
        {
            return 1;
        }
        if (flags & misaka_device_soft_i2c_RD)
        {
            misaka_device_soft_i2c_restart(ops);
            addr1 |= 0x01;
            ret = misaka_device_soft_i2c_send_address(ops, addr1, retries);
            if ((ret != 1) && !ignore_nack)
            {
                return 1;
            }
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & misaka_device_soft_i2c_RD)
        {
            addr1 |= 1;
        }
        ret = misaka_device_soft_i2c_send_address(ops, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 内部操作函数
 * @param  ops              i2c设备
 * @param  msgs             消息对象
 * @param  num              消息数量
 * @return uint16_t @c 操作的消息数
 */
static uint16_t misaka_device_soft_i2c_bit_xfer(misaka_device_soft_i2c_t ops, misaka_device_soft_i2c_message* msgs, uint16_t num)
{
    misaka_device_soft_i2c_message_t msg;
    uint32_t i, ret;
    uint16_t ignore_nack;

    misaka_device_soft_i2c_start(ops);
    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];

        ignore_nack = msg->flags & misaka_device_soft_i2c_IGNORE_NACK;

        if (!(msg->flags & misaka_device_soft_i2c_NO_START))
        {
            if (i)
            {
                misaka_device_soft_i2c_restart(ops);
            }
            ret = misaka_device_soft_i2c_bit_send_address(ops, msg);
            if ((ret != 0) && !ignore_nack)
            {
                goto out;
            }
        }
        if (msg->flags & misaka_device_soft_i2c_RD)
        {
            ret = misaka_device_soft_i2c_recv_bytes(ops, msg);
            if (ret < msg->len)
            {
                if (ret >= 0)
                {
                    ret = 0;
                }
                goto out;
            }
        }
        else
        {
            ret = misaka_device_soft_i2c_send_bytes(ops, msg);
            if (ret < msg->len)
            {
                if (ret >= 0)
                {
                    ret = 0;
                }
                goto out;
            }
        }
    }
    ret = i;

out:
    misaka_device_soft_i2c_stop(ops);

    return ret;
}

/**
 * @brief 外部操作函数
 * @param  ops              i2c设备
 * @param  msgs             消息对象
 * @param  num              消息数量
 * @return uint16_t @c 操作的消息数
 */
uint16_t misaka_device_soft_i2c_transfer(misaka_device_soft_i2c_t ops, misaka_device_soft_i2c_message* msgs, uint16_t num)
{
    uint16_t ret;

    ops->mutex_take();
    ret = misaka_device_soft_i2c_bit_xfer(ops, msgs, num);
    ops->mutex_release();
    if (ret != num)
    {
        misaka_device_soft_i2c_error_callback(ops);
    }

    return ret;
}

/**
 * @brief 发送数据
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  flags            标志
 * @param  buf              待发送数据
 * @param  len              发送数据长度
 * @return uint16_t @c 操作的消息数
 */
uint16_t misaka_device_soft_i2c_master_send(misaka_device_soft_i2c_t ops, uint16_t addr, uint16_t flags, uint8_t* buf, uint32_t len)
{
    uint16_t ret;
    misaka_device_soft_i2c_message msg;

    msg.addr = addr;
    msg.flags = flags;
    msg.len = len;
    msg.buf = (uint8_t*) buf;

    ret = misaka_device_soft_i2c_transfer(ops, &msg, 1);

    return ret;
}

/**
 * @brief 接收数据
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  flags            标志
 * @param  rxbuf            待接收数据
 * @param  rxlen            接收数据长度
 * @return uint16_t @c 操作的消息数
 */
uint16_t misaka_device_soft_i2c_master_recv(misaka_device_soft_i2c_t ops, uint16_t addr, uint16_t flags, uint8_t* rxbuf, uint32_t rxlen)
{
    uint16_t ret;
    misaka_device_soft_i2c_message msg;

    msg.addr = addr;
    msg.flags = flags | misaka_device_soft_i2c_RD;
    msg.len = rxlen;
    msg.buf = rxbuf;

    ret = misaka_device_soft_i2c_transfer(ops, &msg, 1);

    return ret;
}

/**
 * @brief 发送数据后接收数据
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  txbuf            待发送数据
 * @param  txlen            发送数据长度
 * @param  rxbuf            待接收数据
 * @param  rxlen            接收数据长度
 * @return uint16_t @c 操作的消息数
 */
uint16_t misaka_device_soft_i2c_master_send_then_recv(misaka_device_soft_i2c_t ops, uint16_t addr, uint8_t* txbuf, uint32_t txlen, uint8_t* rxbuf, uint32_t rxlen)
{
    uint16_t ret;
    misaka_device_soft_i2c_message msg[2];

    msg[0].addr = addr;
    msg[0].flags = misaka_device_soft_i2c_WR;
    msg[0].buf = txbuf;
    msg[0].len = txlen;

    msg[1].addr = addr;
    msg[1].flags = misaka_device_soft_i2c_RD;
    msg[1].buf = rxbuf;
    msg[1].len = rxlen;

    ret = misaka_device_soft_i2c_transfer(ops, msg, 2);

    return ret;
}

/**
 * @brief 发送数据后发送数据
 * @param  ops              i2c设备
 * @param  addr             地址
 * @param  txbuf1           待发送数据1
 * @param  txlen1           发送数据长度1
 * @param  txbuf2           待发送数据2
 * @param  txlen2           发送数据长度2
 * @return uint16_t @c 操作的消息数
 */
uint16_t misaka_device_soft_i2c_master_send_then_send(misaka_device_soft_i2c_t ops, uint16_t addr, uint8_t* txbuf1, uint32_t txlen1, uint8_t* txbuf2, uint32_t txlen2)
{
    uint16_t ret;
    misaka_device_soft_i2c_message msg[2];

    msg[0].addr = addr;
    msg[0].flags = misaka_device_soft_i2c_WR;
    msg[0].buf = txbuf1;
    msg[0].len = txlen1;

    msg[1].addr = addr;
    msg[1].flags = misaka_device_soft_i2c_WR | misaka_device_soft_i2c_NO_START;
    msg[1].buf = txbuf2;
    msg[1].len = txlen2;

    ret = misaka_device_soft_i2c_transfer(ops, msg, 2);

    return ret;
}

/**
 * @brief 初始化函数
 * @param  ops              i2c设备
 */
void misaka_device_soft_i2c_init(misaka_device_soft_i2c_t ops)
{
    misaka_device_soft_i2c_ASSERT(ops);
    misaka_device_soft_i2c_ASSERT(ops->delay_us);
    misaka_device_soft_i2c_ASSERT(ops->set_scl);
    misaka_device_soft_i2c_ASSERT(ops->set_sda);
    misaka_device_soft_i2c_ASSERT(ops->set_sda_out);
    misaka_device_soft_i2c_ASSERT(ops->get_sda);
    misaka_device_soft_i2c_ASSERT(ops->set_sda_in);
    misaka_device_soft_i2c_ASSERT(ops->mutex_release);
    misaka_device_soft_i2c_ASSERT(ops->mutex_take);
}