// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>


#define MAX_SUPPORTED_ALT_FUN_VIPER 14
#define MAX_SUPPORTED_ALT_FUN_LAN8814  16

#define MALIBU10G_EVENT           2
#define MALIBU10G_EXTENDED_EVENT  3
#define MALIBU10G_EXTENDED2_EVENT 4
#define MALIBU10G_MACSEC_EVENT    5
#define MALIBU10G_PTP_EVENT       6

#define GPIO_NUMBER_31            (31U)

typedef struct {
    uint8_t      gpio_number;
    mepa_bool_t  intr_a;
} gpio_configuration;


typedef struct {
    uint8_t      alt_mode;
    uint8_t      gpio_no;
    const char   *desc;
} gpio_table_t;

gpio_table_t viper[MAX_SUPPORTED_ALT_FUN_VIPER] = {//Alt mode    // gpio_no    // Alt Funct
    {0,               0,        "Signal Detect 0"},
    {1,               1,        "Signal Detect 1"},
    {2,               2,        "Signal Detect 2"},
    {3,               3,        "Signal Detect 3"},
    {4,               4,        "I2C Clk 0"},
    {5,               5,        "I2C Clk 1"},
    {6,               6,        "I2C Clk 2"},
    {7,               7,        "I2C Clk 3"},
    {8,               8,        "I2C Sda"},
    {9,               9,        "Fast Link Failure"},
    {10,              10,       "1588 load Save"},
    {11,              11,       "1588 PPS"},
    {12,              12,       "1588 SPI CS"},
    {13,              13,       "1588 SPI DO"},
};

gpio_table_t lan8814[MAX_SUPPORTED_ALT_FUN_LAN8814] = {//Alt mode    // gpio_no    // Alt Funct
    {0,               0,        "1588 Event A"},
    {1,               1,        "1588 Event B"},
    {2,               2,        "1588 Ref Clk"},
    {3,               3,        "1588 LD ADJ"},
    {4,               4,        "1588 STI CS N"},
    {5,               5,        "1588 STI CLK"},
    {6,               6,        "1588 STI DO"},
    {7,               7,        "Rcvrd Clk In 1"},
    {8,               8,        "Rcvrd Clk In 2"},
    {9,               9,        "Rcvrd Clk Out 1"},
    {10,              10,       "Rcvrd Clk Out 2"},
    {11,              15,       "SOF 0"},
    {12,              21,       "SOF 1"},
    {13,              16,       "SOF 2"},
    {14,              23,       "SOF 3"},
    {15,              11,       "Port Led"},
};

gpio_table_t malibu10g_ch_0[8] = {//Alt mode         //gpio_no      //Alt Function
    {0,                0,           "Rate Select"},
    {1,                1,           "Module Absent"},
    {2,                2,           "I2C Master Clock"},
    {3,                3,           "I2C Master Data"},
    {4,                4,           "Tx Disable"},
    {5,                5,           "Tx Fault"},
    {6,                6,           "Rx LOS"},
    {7,                7,           "Link Up"},
};
