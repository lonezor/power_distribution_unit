/*
 *  PDU
 *  Copyright (C) 2025 Johan Norberg <lonezor@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/input.h>

#include <rc/relay_32.h>

//-------------------------------------------------------------------------------------------------------------------

/*
 * Overview:
 *
 * This implementation is based on the relay modules made by
 * National Control Devices. They provide an I2C interface that
 * connects to the MCP23017 module from Microchip Technology.
 *
 * The relay channels are controlled by configuring the I/O
 * direction register to output mode and setting the associated
 * GPIO pins.
 *
 * For 32 port relay modules, two I2C addresses are used. The
 * implementation also supports a 16 port relay module using
 * only one address.
 */

#define I2C_DEV_BASE_PATH "/dev/i2c-1"
#define I2C_ADDRESS_RELAY_MODULE_PORT_A (0x20)
#define I2C_ADDRESS_RELAY_MODULE_PORT_B (0x21)
#define CONTROL_REGISTER_IODIR (0x00) // word: 0x00 - 0x01
#define CONTROL_REGISTER_GPIO (0x12)  // word: 0x12 - 0x13

/******************************************************************
 *********************** COMMAND LINE TOOLS ***********************
 ******************************************************************
 *
 * Discovery:
 * i2cdetect -l
 * i2cdetect -y 1
 *
 * Dump all memory:
 * i2cdump -y 1 0x20
 * i2cdump -y 1 0x21
 *
 * Data direction register (0xff: input direction) and GPIO pins:
 * i2cset -y 1 0x20 0x00 0x00
 * i2cset -y 1 0x20 0x01 0x00
 * i2cset -y 1 0x20 0x12 0xff
 * i2cset -y 1 0x20 0x13 0xff
 *
 * i2cset -y 1 0x21 0x00 0x00
 * i2cset -y 1 0x21 0x01 0x00
 * i2cset -y 1 0x21 0x12 0xff
 * i2cset -y 1 0x21 0x13 0xff
 */

//-------------------------------------------------------------------------------------------------------------------

static int open_dev(rc_relay_port_t port)
{
    int fd = open(I2C_DEV_BASE_PATH, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "error: open I2C device failed\n");
        return -1;
    }

    unsigned long request = I2C_ADDRESS_RELAY_MODULE_PORT_A;
    if (port == rc_relay_port_b) {
        request = I2C_ADDRESS_RELAY_MODULE_PORT_B;
    }

    if (ioctl(fd, I2C_SLAVE, request) < 0) {
        close(fd);
        fprintf(stderr, "error: configuring I2C slave address failed\n");
        return -1;
    }

    return fd;
}

//-------------------------------------------------------------------------------------------------------------------

static uint16_t rc_read_relay_state(rc_relay_port_t port)
{
    uint16_t state = 0;

    int fd = open_dev(port);
    if (fd == -1) {
        return 0;
    }

    struct i2c_smbus_ioctl_data blk;
    union i2c_smbus_data i2cdata;

    blk.read_write = 1;
    blk.command = CONTROL_REGISTER_GPIO;
    blk.size = I2C_SMBUS_I2C_BLOCK_DATA;
    blk.data = &i2cdata;
    i2cdata.block[0] = sizeof(uint16_t);

    if (ioctl(fd, I2C_SMBUS, &blk) < 0) {
        close(fd);
        fprintf(stderr, "error: reading I2C bus failed\n");
        return 0;
    }

    memcpy(&state, &i2cdata.block[1], sizeof(uint16_t));

    close(fd);

    return state;
}

//-------------------------------------------------------------------------------------------------------------------

static void rc_write_relay_state(rc_relay_port_t port, uint8_t cmd,
                                 uint16_t state)
{
    int fd = open_dev(port);
    if (fd == -1) {
        return;
    }

    struct i2c_smbus_ioctl_data blk;
    union i2c_smbus_data i2cdata;

    i2cdata.word = state;
    blk.read_write = 0;
    blk.command = cmd;
    blk.size = I2C_SMBUS_WORD_DATA;
    blk.data = &i2cdata;

    if (ioctl(fd, I2C_SMBUS, &blk) < 0) {
        close(fd);
        fprintf(stderr, "error: writing I2C bus failed\n");
        return;
    }

    close(fd);
}

//-------------------------------------------------------------------------------------------------------------------

void rc_relay_channel_init()
{
    rc_write_relay_state(rc_relay_port_a, CONTROL_REGISTER_IODIR, 0x0000);
    rc_write_relay_state(rc_relay_port_b, CONTROL_REGISTER_IODIR, 0x0000);
}

//-------------------------------------------------------------------------------------------------------------------

void rc_relay_channel_set(rc_relay_port_t port, rc_relay_channel_t ch,
                          bool enabled)
{
    // Get state for all channels (API bitfield representation)
    uint16_t state = rc_read_relay_state(port);

    // Update channel specific bit in the bitfield
    if (enabled) {
        state |= (uint16_t)ch;
    } else {
        state &= ~(uint16_t)ch;
    }

    rc_write_relay_state(port, CONTROL_REGISTER_GPIO, state);
}

//-------------------------------------------------------------------------------------------------------------------

bool rc_relay_channel_get(rc_relay_port_t port, rc_relay_channel_t ch)
{
    // Get state for all channels (API bitfield representation)
    uint16_t state = rc_read_relay_state(port);

    // Determine if requested channel(s) are enabled (completely)
    uint16_t res = (state & (uint16_t)ch);

    return res > 0;
}

//-------------------------------------------------------------------------------------------------------------------
