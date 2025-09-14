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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rc/relay_32.h>

#define PROGRAM_VERSION "relay_ctrl v0.6"

//-------------------------------------------------------------------------------------------------------------------

enum cli_option
{
    cli_option_list_commands = 1000, // not colliding with ASCII range
    cli_option_relay_command,
    cli_option_help,
    cli_option_version,
};

//-------------------------------------------------------------------------------------------------------------------

static bool g_help = false;
static bool g_version = false;
static bool g_list_commands = false;
static rc_relay_port_t g_relay_port = rc_relay_port_a;
static rc_relay_channel_t g_relay_channel = rc_relay_channel_none;
static bool g_relay_set = true;
static bool g_relay_enabled = false;

//-------------------------------------------------------------------------------------------------------------------

static struct option long_options[] = {
    {"list-commands", no_argument, NULL, cli_option_list_commands},
    {"command", required_argument, NULL, cli_option_relay_command},
    {"help", no_argument, NULL, cli_option_help},
    {"version", no_argument, NULL, cli_option_version},
    {NULL, 0, NULL, 0}};

//-------------------------------------------------------------------------------------------------------------------

static void print_commands()
{
    printf("Available commands:\n");
    printf(" - relay_01  <on/off/query>\n");
    printf(" - relay_02  <on/off/query>\n");
    printf(" - relay_03  <on/off/query>\n");
    printf(" - relay_04  <on/off/query>\n");
    printf(" - relay_05  <on/off/query>\n");
    printf(" - relay_06  <on/off/query>\n");
    printf(" - relay_07  <on/off/query>\n");
    printf(" - relay_08  <on/off/query>\n");
    printf(" - relay_09  <on/off/query>\n");
    printf(" - relay_10  <on/off/query>\n");
    printf(" - relay_11  <on/off/query>\n");
    printf(" - relay_12  <on/off/query>\n");
    printf(" - relay_13  <on/off/query>\n");
    printf(" - relay_14  <on/off/query>\n");
    printf(" - relay_15  <on/off/query>\n");
    printf(" - relay_16  <on/off/query>\n");
    printf(" - relay_17  <on/off/query>\n");
    printf(" - relay_18  <on/off/query>\n");
    printf(" - relay_19  <on/off/query>\n");
    printf(" - relay_20  <on/off/query>\n");
    printf(" - relay_21  <on/off/query>\n");
    printf(" - relay_22  <on/off/query>\n");
    printf(" - relay_23  <on/off/query>\n");
    printf(" - relay_24  <on/off/query>\n");
    printf(" - relay_25  <on/off/query>\n");
    printf(" - relay_26  <on/off/query>\n");
    printf(" - relay_27  <on/off/query>\n");
    printf(" - relay_28  <on/off/query>\n");
    printf(" - relay_29  <on/off/query>\n");
    printf(" - relay_30  <on/off/query>\n");
    printf(" - relay_31  <on/off/query>\n");
    printf(" - relay_32  <on/off/query>\n");
    printf("\n");
}

//-------------------------------------------------------------------------------------------------------------------

static void print_help()
{
    // clang-format off
    printf("Usage: relay_ctrl [OPTIONS]\n");
    printf("\n");
    printf("Options (in order of importance):\n");
    printf(" -l --list-commands    List available commands\n");
    printf(" -c --command=STRING   Run command\n");
    printf(" -h --help             This help screen\n");
    printf("    --version          Prints version and exits\n");
    printf("\n");
    // clang-format on
}

//-------------------------------------------------------------------------------------------------------------------

static void print_version() { printf("%s\n", PROGRAM_VERSION); }

//-------------------------------------------------------------------------------------------------------------------

static bool parse_arguments(int argc, char *argv[])
{
    // Parse command line arguments
    int c;
    int option_index = 0;
    while (true) {
        // Using getopt_long API -
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        c = getopt_long(argc, argv, "lc:h", long_options, &option_index);

        // All options parsed
        if (c == -1) {
            break;
        }

        switch (c) // Listed in order of importance
        {
        case 'l':
        case cli_option_list_commands:
            g_list_commands = true;
            break;
        case 'c':
        case cli_option_relay_command: {
            // Get command context
            if (strstr(optarg, "query")) {
                g_relay_set = false;
            } else { // set
                g_relay_enabled = strstr(optarg, "on") != NULL;
            }

            // Get channel (it is small enough, so no need to parse number)
            if (strstr(optarg, "relay_01")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_01;
            } else if (strstr(optarg, "relay_02")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_02;
            } else if (strstr(optarg, "relay_03")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_03;
            } else if (strstr(optarg, "relay_04")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_04;
            } else if (strstr(optarg, "relay_05")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_05;
            } else if (strstr(optarg, "relay_06")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_06;
            } else if (strstr(optarg, "relay_07")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_07;
            } else if (strstr(optarg, "relay_08")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_08;
            } else if (strstr(optarg, "relay_09")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_09;
            } else if (strstr(optarg, "relay_10")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_10;
            } else if (strstr(optarg, "relay_11")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_11;
            } else if (strstr(optarg, "relay_12")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_12;
            } else if (strstr(optarg, "relay_13")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_13;
            } else if (strstr(optarg, "relay_14")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_14;
            } else if (strstr(optarg, "relay_15")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_15;
            } else if (strstr(optarg, "relay_16")) {
                g_relay_port = rc_relay_port_a;
                g_relay_channel = rc_relay_channel_16;
            } else if (strstr(optarg, "relay_17")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_01;
            } else if (strstr(optarg, "relay_18")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_02;
            } else if (strstr(optarg, "relay_19")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_03;
            } else if (strstr(optarg, "relay_20")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_04;
            } else if (strstr(optarg, "relay_21")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_05;
            } else if (strstr(optarg, "relay_22")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_06;
            } else if (strstr(optarg, "relay_23")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_07;
            } else if (strstr(optarg, "relay_24")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_08;
            } else if (strstr(optarg, "relay_25")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_09;
            } else if (strstr(optarg, "relay_26")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_10;
            } else if (strstr(optarg, "relay_27")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_11;
            } else if (strstr(optarg, "relay_28")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_12;
            } else if (strstr(optarg, "relay_29")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_13;
            } else if (strstr(optarg, "relay_30")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_14;
            } else if (strstr(optarg, "relay_31")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_15;
            } else if (strstr(optarg, "relay_32")) {
                g_relay_port = rc_relay_port_b;
                g_relay_channel = rc_relay_channel_16;
            }
            break;
        }
        case cli_option_help:
            g_help = true;
            break;
        case cli_option_version:
            g_version = true;
            break;
        default:
            continue;
        }
    }

    if (g_help || g_version || g_list_commands) {
        return true;
    }

    if (g_relay_channel == rc_relay_channel_none) {
        fprintf(stderr,
                "error: No command given! (--help for more information)\n");
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    // Parse mandatory and optional arguments
    if (!parse_arguments(argc, argv)) {
        return EXIT_FAILURE;
    }

    // Print Help Screen and exit
    if (g_help) {
        print_help();
        return EXIT_SUCCESS;
    }

    // Print Version Screen and exit
    if (g_version) {
        print_version();
        return EXIT_SUCCESS;
    }

    if (g_list_commands) {
        print_commands();
        return EXIT_SUCCESS;
    }

    if (g_relay_set) {
        rc_relay_channel_init();
        rc_relay_channel_set(g_relay_port, g_relay_channel, g_relay_enabled);
    } else { // query
        bool enabled = rc_relay_channel_get(g_relay_port, g_relay_channel);
        printf("value: %s\n", enabled ? "true" : "false");
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
