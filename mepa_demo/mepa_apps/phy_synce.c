// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "microchip/ethernet/phy/api/types.h"

#include "vtss_phy_10g_api.h"
#include "vtss_private.h"
#include "lan80xx.h"

#include "main.h"
#include "mesa-rpc.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "mepa_driver.h"
#include "phy_demo_apps.h"

#define GPIO_6  6
#define GPIO_14 14
#define GPIO_22 22
#define GPIO_30 30

meba_inst_t meba_phy_instances;

static int cli_parm_clk_src (cli_req_t *req);
static int cli_parm_freq (cli_req_t *req);
static int cli_parm_clk_out (cli_req_t *req);
static void cli_cmd_synce_conf_set (cli_req_t *req);
static void cli_cmd_chip_specific_synce_conf(cli_req_t *req);

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_synce_config"
};

enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};

static mscc_appl_trace_group_t trace_groups[10] = {
    // TRACE_GROUP_DEFAULT
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

typedef enum {
    SYNCE_CLOCK_SRC_DISABLED,
    SYNCE_CLOCK_SRC_SERDES_MEDIA,
    SYNCE_CLOCK_SRC_COPPER_MEDIA,
    SYNCE_CLOCK_SRC_CLOCK_IN_1,
    SYNCE_CLOCK_SRC_CLOCK_IN_2,
} synce_clock_src_t;

typedef enum {
    SYNCE_CLOCK_DST_1 = 0,
    SYNCE_CLOCK_DST_2
} synce_clock_dst_t;

typedef enum {
    LAN8814_FREQ_25M,    /**< 25Mhz recovered clock */
    LAN8814_FREQ_31_25M, /**< 31.25Mhz receovered clock */
    LAN8814_FREQ_125M,   /**< 125Mhz recovered clock */
    PHY_10G_SCKOUT_156_25,  /**< 156,25 MHz */
    PHY_10G_SCKOUT_125_00,  /**< 125,00 MHz */
    PHY25G_RCKOUT_75_00,  /**< 75 MHz */
    PHY25G_RCKOUT_37_50,  /**< 37.50 MHz */
    PHY25G_RCKOUT_79_58,  /**< 79.58 MHz */
    PHY25G_RCKOUT_39_79,  /**< 39.79 MHz */
    PHY25G_RCKOUT_125_00,  /**< 125 MHz */
    PHY25G_RCKOUT_62_50,  /**< 62.50 MHz */
    PHY25G_RCKOUT_31_25,  /**< 31.25 MHz */
    PHY25G_RCKOUT_15_625,  /**< 15.625 MHz */
    PHY25G_RCKOUT_128_90625,  /**< 128.90625 MHz */
    PHY25G_RCKOUT_64_453125,  /**< 64.453125 MHz */
    PHY25G_RCKOUT_32_2265625,  /**< 32.2265625 MHz */
    PHY25G_RCKOUT_80_56640625,  /**< 80.56640625 MHz */
} freq_t;

typedef struct {
    synce_clock_src_t src; /**< clock source */
    synce_clock_dst_t dst; /**< recovered clock output A/B */
    freq_t            freq;/**< recovered clock out frequency */
} port_synce_conf_set_t;

static cli_cmd_t cli_cmd_table[] = {
    {
        "synce_conf_set <port_no> [disableclk|serdes|coppermedia|clk1|clk2] [25mhz|31.25mhz|125mhz|156.25mhz] [clkouta|clkoutb]",
        "syncE configuration set for particular port number",
        cli_cmd_synce_conf_set
    },
    {
        "chip_specific_synce <port_no> [-f] [filename]",
        "Chip specific syncE configuration set for particular port number",
        cli_cmd_chip_specific_synce_conf
    },

};

static cli_parm_t cli_parm_table[] = {
    {
        "disableclk|serdes|coppermedia|clk1|clk2",
        "\n\t DISABLECLK	: Disable clock source\n"
        "\t SERDES		: Serdes clock source\n"
        "\t COPPERMEDIA	: Coppermedia clock source\n"
        "\t CLK1		: Clock source1\n"
        "\t CLK2		: Clock source2\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_clk_src
    },
    {
        "25mhz|31.25mhz|125mhz|156.25mhz",
        "\n\t 25MHZ		: 25mhz expected clock frequency\n"
        "\t 31.25MHZ	: 31.25mhz expected clock frequency\n"
        "\t 125MHZ		: 125mhz expected clock frequency\n"
        "\t 156.25MHZ	: 156.25mhz expected clock frequency\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_freq
    },
    {
        "clkouta|clkoutb",
        "\n\t CLKOUTA	: Recovered clock output A\n"
        "\t CLKOUTB	: Recovered clock output B\n",
        CLI_PARM_FLAG_SET,
        cli_parm_clk_out
    },
};
static void cli_cmd_synce_conf_set(cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    mepa_phy_info_t phy_info = {0};
    mepa_synce_clock_conf_t lan8814_synce_conf;
    vtss_phy_10g_sckout_conf_t phy10g_synce_conf;
    struct mepa_device *dev = meba_phy_instances->phy_devices[req->port_no];
    phy_25g_rckout_conf_t phy25g_synce_conf;
    demo_phy_info_t phy_family;
    mepa_rc rc;

    if (dev == NULL) {
        T_E("\n Not phy is connected on this port number\n");
        return;
    }
    if (!req->set) {
        cli_printf("\n Syntax : synce_conf_set [<port_no> <clk_src> <freq> <clk-out>] \n");
        T_E("\n Provide Paramter clock source/frequency/clock output\n");
        return;
    }
    phy_data_t *data = (phy_data_t *)dev->data;
    meba_phy_info_get(meba_phy_instances, req->port_no, &phy_info);
    if ((rc = phy_family_detect(meba_phy_instances, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if ((phy_family.family == PHY_FAMILY_LAN8814) || (phy_family.family == PHY_FAMILY_VIPER)) {
        /* lan8814 PHY or VIPER PHY*/
        if (mreq->src > SYNCE_CLOCK_SRC_CLOCK_IN_2) {
            T_E("\n Provide valid clock source for the PHY \n");
            return;
        }
        if ((mreq->freq > LAN8814_FREQ_125M) && (mreq->freq < LAN8814_FREQ_25M)) {
            T_E("\n Provide valid clock frequency for the PHY \n");
            return;
        }
        if (mreq->dst > SYNCE_CLOCK_DST_2) {
            T_E("\n Provide valid clock output for the PHY \n");
            return;
        }
        lan8814_synce_conf.src = mreq->src;
        lan8814_synce_conf.freq = mreq->freq;
        lan8814_synce_conf.dst = mreq->dst;
        if ((meba_phy_synce_clock_conf_set(meba_phy_instances, req->port_no, &lan8814_synce_conf)) != MESA_RC_OK) {
            T_E("\nlan8814: Error setting Synce configuration \n");
            return;
        }
    } else if (phy_family.family == PHY_FAMILY_MALIBU_10G) {
        /* MALIBU10G PHY*/
        switch (mreq->src) {
        case SYNCE_CLOCK_SRC_DISABLED:
            phy10g_synce_conf.mode = VTSS_PHY_10G_SYNC_DISABLE;
            phy10g_synce_conf.enable = FALSE;
            break;
        case SYNCE_CLOCK_SRC_SERDES_MEDIA:
            phy10g_synce_conf.mode = VTSS_PHY_10G_LINE0_RECVRD_CLOCK;
            break;
        case SYNCE_CLOCK_SRC_COPPER_MEDIA:
            T_E("\n Copper media not supported for the PHY \n");
            return;
        case SYNCE_CLOCK_SRC_CLOCK_IN_1:
            phy10g_synce_conf.mode = VTSS_PHY_10G_SREFCLK;
            break;
        case SYNCE_CLOCK_SRC_CLOCK_IN_2:
            T_E("\n Not supported for the PHY \n");
            return;
        default:
            T_E("\n Provide valid clock source for the PHY \n");
            return;
        }
        if (mreq->freq == LAN8814_FREQ_125M) {
            mreq->freq = PHY_10G_SCKOUT_125_00;
        }
        if ((mreq->freq != PHY_10G_SCKOUT_125_00) && (mreq->freq != PHY_10G_SCKOUT_156_25)) {
            T_E("\n Provide valid clock frequency for the PHY \n");
            return;
        }
        if (mreq->dst != SYNCE_CLOCK_DST_1) {
            T_E("\n Provide valid clock output for the PHY \n");
            return;
        }
        phy10g_synce_conf.freq = mreq->freq - 3;
        phy10g_synce_conf.src = VTSS_CKOUT_NO_SQUELCH;
        if (mreq->src != SYNCE_CLOCK_SRC_DISABLED) {
            phy10g_synce_conf.enable = TRUE;
        }
        phy10g_synce_conf.squelch_inv = FALSE;
        if ((vtss_phy_10g_sckout_conf_set(data->vtss_instance, req->port_no, &phy10g_synce_conf)) != MESA_RC_OK) {
            T_E("\nMALIBU10G: Error setting Synce configuration \n");
            return;
        }
    } else if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        /* MALIBU25G PHY*/
        switch (mreq->src) {
        case SYNCE_CLOCK_SRC_DISABLED:
            phy10g_synce_conf.src = LAN80XX_NONE;
            break;
        case SYNCE_CLOCK_SRC_SERDES_MEDIA:
            phy10g_synce_conf.src = LAN80XX_LINE0_RECVRD_CLOCK;
            break;
        case SYNCE_CLOCK_SRC_COPPER_MEDIA:
            T_E("\n Copper media not supported for the PHY \n");
            return;
        case SYNCE_CLOCK_SRC_CLOCK_IN_1:
            phy10g_synce_conf.src = LAN80XX_SREFCLK;
            break;
        case SYNCE_CLOCK_SRC_CLOCK_IN_2:
            T_E("\n Not supported for the PHY \n");
            return;
        default:
            T_E("\n Provide valid clock source for the PHY \n");
            return;
        }
        if (mreq->freq == LAN8814_FREQ_125M) {
            mreq->freq = PHY25G_RCKOUT_125_00;
        } else if (mreq->freq == LAN8814_FREQ_31_25M) {
            mreq->freq = PHY25G_RCKOUT_31_25;
        }
        if ((mreq->freq >= PHY25G_RCKOUT_80_56640625) || (mreq->freq <= PHY25G_RCKOUT_75_00)) {
            T_E("\n Provide valid clock frequency for the PHY \n");
            return;
        }
        if (mreq->dst > SYNCE_CLOCK_DST_2) {
            T_E("\n Provide valid clock output for the PHY \n");
            return;
        }
        phy25g_synce_conf.freq = mreq->freq - 5;
        phy25g_synce_conf.rckout_sel = mreq->dst;
        phy25g_synce_conf.rcvd_clk_enable = TRUE;
        phy25g_synce_conf.los_squelch_enable = FALSE;
        phy25g_synce_conf.link_sts_enable = FALSE;
        phy25g_synce_conf.auto_squelch_enable = FALSE;
        if ((lan80xx_rckout_conf_set(dev, req->port_no, &phy25g_synce_conf)) != MESA_RC_OK) {
            T_E("\nMALIBU25G: Error setting Synce configuration \n");
            return;
        }
    } else {
        T_E("SyncE not supported for this PHY\n");
        return;
    }
    cli_printf("SyncE configured for the PHY\n");
}

static void cli_cmd_chip_specific_synce_conf(cli_req_t *req)
{
    mepa_synce_clock_conf_t lan8814_synce_conf;
    vtss_phy_10g_sckout_conf_t phy10g_synce_conf;
    struct json_object *jobj = NULL;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    int size = 0;
    char synce_config_file[128];
    char *buffer = NULL;
    demo_phy_info_t phy_family;
    struct mepa_device *dev = meba_phy_instances->phy_devices[req->port_no];
    phy_25g_rckout_conf_t phy25g_synce_conf;
    mepa_gpio_conf_t gpio_conf;
    mepa_conf_t config;
    uint8_t gpio_no[4] = {GPIO_6, GPIO_14, GPIO_22, GPIO_30};
    mepa_rc rc = MEPA_RC_OK;

    if (dev == NULL) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if (!req->set) {
        cli_printf("\n Syntax: chip_specific_synce <port_no> [-f] [filename]\n");
        T_E("\n Provide Port no and filename as arguments\n");
        return;
    }
    if ((rc = phy_family_detect(meba_phy_instances, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    phy_data_t *data = (phy_data_t *)dev->data;
    snprintf(synce_config_file, sizeof(synce_config_file), "/root/mepa_scripts/%s", req->file_name);
    FILE *fp = fopen(synce_config_file, "r");
    if (fp == NULL) {
        T_E("%s : file open error. Provide Valid Filename: synce_config.json", req->file_name);
        return;
    }
    if (!(fseek(fp, 0, SEEK_END)) && ftell(fp)) {
        size = ftell(fp);
        buffer = (char *)malloc(size);
        if (buffer == NULL) {
            T_E("Fatal: failed to allocate %d bytes.\n", size);
            goto file_close;
        }
        fseek(fp, 0, SEEK_SET);
        if (!fread(buffer, 1, size, fp)) {
            T_E("File read error");
            goto file_close;
        }
        jobj = json_tokener_parse(buffer);
        if (jobj == NULL) {
            T_E("could not parse parms from file: %s", req->file_name);
            goto file_close;
        }
        if ((phy_family.family == PHY_FAMILY_LAN8814) || (phy_family.family == PHY_FAMILY_VIPER)) {
            if (json_rpc_get_name_json_object(&json_req, jobj, "1g_synce", &json_req.params) != MESA_RC_OK) {
                T_E("object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if (json_rpc_get_mepa_synce_clock_conf_t(&json_req, json_req.params, &lan8814_synce_conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if (meba_phy_synce_clock_conf_set(meba_phy_instances, req->port_no, &lan8814_synce_conf) != MESA_RC_OK) {
                T_E("Error in Configuring the SyncE");
            }
            goto file_close;

        } else if (phy_family.family == PHY_FAMILY_MALIBU_10G) {
            if (json_rpc_get_name_json_object(&json_req, jobj, "10g_synce", &json_req.params) != MESA_RC_OK) {
                T_E("object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if (json_rpc_get_vtss_phy_10g_sckout_conf_t(&json_req, json_req.params, &phy10g_synce_conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if (vtss_phy_10g_sckout_conf_set(data->vtss_instance, req->port_no, &phy10g_synce_conf) != MESA_RC_OK) {
                T_E("Error in Configuring the SyncE");
            }
            goto file_close;

        } else if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
            if (json_rpc_get_name_json_object(&json_req, jobj, "25g_synce", &json_req.params) != MESA_RC_OK) {
                T_E("object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if (json_rpc_get_phy_25g_rckout_conf_t(&json_req, json_req.params, &phy25g_synce_conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if ((meba_phy_conf_get(meba_phy_instances, req->port_no, &config)) != MESA_RC_OK) {
                T_E("\nMALIBU25G: Error in getting configuration\n");
                goto file_close;
            }
            /* GPIO should held in alternate mode for LOS Squelch */
            gpio_conf.gpio_no = gpio_no[config.conf_25g.channel_id - 1];
            gpio_conf.led_num = 0;
            gpio_conf.mode =  phy25g_synce_conf.los_squelch_enable ? MEPA_GPIO_MODE_ALT : MEPA_GPIO_MODE_IN;
            if (meba_phy_gpio_mode_set(meba_phy_instances, req->port_no, &gpio_conf) != MESA_RC_OK) {
                T_E("\nMALIBU25G: Error in setting GPIO %d as Alt/Input mode \n", gpio_conf.gpio_no);
            }
            if ((lan80xx_rckout_conf_set(dev, req->port_no, &phy25g_synce_conf)) != MESA_RC_OK) {
                T_E("\nMALIBU25G: Error in setting Synce configuration \n");
            }
            goto file_close;
        }

file_close:
        free(buffer);
        fclose(fp);
    }
}

static int cli_parm_clk_src (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        return 1;
    }
    if (!strncasecmp(found, "disableclk", 10)) {
        mreq->src = SYNCE_CLOCK_SRC_DISABLED;
    } else if (!strncasecmp(found, "serdes", 6)) {
        mreq->src = SYNCE_CLOCK_SRC_SERDES_MEDIA;
    } else if (!strncasecmp(found, "coppermedia", 11)) {
        mreq->src = SYNCE_CLOCK_SRC_COPPER_MEDIA;
    } else if (!strncasecmp(found, "clk1", 4)) {
        mreq->src = SYNCE_CLOCK_SRC_CLOCK_IN_1;
    } else if (!strncasecmp(found, "clk2", 4)) {
        mreq->src = SYNCE_CLOCK_SRC_CLOCK_IN_2;
    } else {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static int cli_parm_freq (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        return 1;
    }
    if (!strncasecmp(found, "25mhz", 5)) {
        mreq->freq = LAN8814_FREQ_25M;
    } else if (!strncasecmp(found, "31.25mhz", 8)) {
        mreq->freq = LAN8814_FREQ_31_25M;
    } else if (!strncasecmp(found, "125mhz", 6)) {
        mreq->freq = LAN8814_FREQ_125M;
    } else if (!strncasecmp(found, "156.25mhz", 9)) {
        mreq->freq = PHY_10G_SCKOUT_156_25;
    }  else if (!strncasecmp(found, "75MHZ", 5)) {
        mreq->freq = PHY25G_RCKOUT_75_00;
    } else if (!strncasecmp(found, "37.50MHZ", 8)) {
        mreq->freq = PHY25G_RCKOUT_37_50;
    } else if (!strncasecmp(found, "79.58MHZ", 8)) {
        mreq->freq = PHY25G_RCKOUT_79_58;
    } else if (!strncasecmp(found, "39.79MHZ", 8)) {
        mreq->freq = PHY25G_RCKOUT_39_79;
    } else if (!strncasecmp(found, "62.50MHZ", 8)) {
        mreq->freq = PHY25G_RCKOUT_62_50;
    } else if (!strncasecmp(found, "15.625MHZ", 9)) {
        mreq->freq = PHY25G_RCKOUT_31_25;
    } else if (!strncasecmp(found, "128.90625MHZ", 11)) {
        mreq->freq = PHY25G_RCKOUT_15_625;
    } else if (!strncasecmp(found, "64.453125MHZ", 12)) {
        mreq->freq = PHY25G_RCKOUT_128_90625;
    } else if (!strncasecmp(found, "32.2265625MHZ", 13)) {
        mreq->freq = PHY25G_RCKOUT_64_453125;
    } else if (!strncasecmp(found, "80.56640625MHZ", 14)) {
        mreq->freq = PHY25G_RCKOUT_80_56640625;
    } else {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static int cli_parm_clk_out (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        return 1;
    }
    if (!strncasecmp(found, "clkouta", 7)) {
        mreq->dst = SYNCE_CLOCK_DST_1;
    } else if (!strncasecmp(found, "clkoutb", 7)) {
        mreq->dst = SYNCE_CLOCK_DST_2;
    } else {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static void phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table) / sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
    /* Register parameters */
    for (i = 0; i < sizeof(cli_parm_table) / sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mscc_appl_phy_synce(mscc_appl_init_t *init)
{
    meba_phy_instances = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;
    default:
        break;
    }
}
