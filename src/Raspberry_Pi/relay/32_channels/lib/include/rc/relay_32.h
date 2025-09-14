/*
 *  PDU
 *  Copyright (C) 2022 Johan Norberg <lonezor@gmail.com>
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /** Relay port */
    typedef enum
    {
        rc_relay_port_a, /** relay port A */
        rc_relay_port_b, /** relay port B */
    } rc_relay_port_t;

    /* @brief Relay channel
     *
     * This describes the relay channels on a logical level using
     * bitfield values. This maps well to the set/get functions.
     *
     * Multiple channels can be described using bitwise OR. For
     * convenience the 'all' channel is covered as well.
     */
    typedef enum
    {
        rc_relay_channel_none = 0x0000,
        rc_relay_channel_01 = 0x0001, /**< Relay channel #01  */
        rc_relay_channel_02 = 0x0002, /**< Relay channel #02  */
        rc_relay_channel_03 = 0x0004, /**< Relay channel #03  */
        rc_relay_channel_04 = 0x0008, /**< Relay channel #04  */
        rc_relay_channel_05 = 0x0010, /**< Relay channel #05  */
        rc_relay_channel_06 = 0x0020, /**< Relay channel #06  */
        rc_relay_channel_07 = 0x0040, /**< Relay channel #07  */
        rc_relay_channel_08 = 0x0080, /**< Relay channel #08  */
        rc_relay_channel_09 = 0x0100, /**< Relay channel #09  */
        rc_relay_channel_10 = 0x0200, /**< Relay channel #10  */
        rc_relay_channel_11 = 0x0400, /**< Relay channel #11  */
        rc_relay_channel_12 = 0x0800, /**< Relay channel #12  */
        rc_relay_channel_13 = 0x1000, /**< Relay channel #13  */
        rc_relay_channel_14 = 0x2000, /**< Relay channel #14  */
        rc_relay_channel_15 = 0x4000, /**< Relay channel #15  */
        rc_relay_channel_16 = 0x8000, /**< Relay channel #16  */
    } rc_relay_channel_t;

    #define RELAY_MSG_MAGIC (0x20141225)

    #pragma pack(push, 1)
    typedef struct {
        uint32_t magic;
        uint8_t  nr_channels;
        uint8_t  port; // only relevant for 32 channel model
        uint16_t channel;
        uint8_t state;
    } relay_cmd_msg_t;
    #pragma pack(pop)

    /* @brief Initialize relay module
     *
     * This function must be called once before invoking rc_relay_channel_set()
     * in order to set the control registers correctly.
     *
     * The control registers are persistent so the init function is not
     * needed when power cycling the relay module. However, for simplicity
     * and since it may have been altered elsewhere it is often best
     * to always invoke it at startup.
     */
    void rc_relay_channel_init();

    /* @brief Set channel value(s)
     *
     * @param ch       Bitfield of relay channels to control
     * @param enabled  Set to true for activated relay
     */
    void rc_relay_channel_set(rc_relay_port_t port, rc_relay_channel_t ch,
                              bool enabled);

    /* @brief Get channel value(s)
     *
     * @param ch       Bitfield of relay channels to control
     *
     * @return True for activated relay(s)
     */
    bool rc_relay_channel_get(rc_relay_port_t port, rc_relay_channel_t ch);

#ifdef __cplusplus
}
#endif
