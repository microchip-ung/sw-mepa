// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT
/* Cross Connect Demo */
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include <vtss_phy_api.h>
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "mesa-rpc.h"
#include "vtss_phy_10g_api.h"
#include "vtss_private.h"
#include "phy_demo_apps.h"
#include "mepa_driver.h"
#include "lan80xx.h"

#define ERROR -1

meba_inst_t meba_xc_phy_instance;

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_port_xconnect"
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

typedef struct {
    mepa_bool_t           file;
    char                  filename[30];
} xconnect_cli_req_t;

typedef struct {
    uint32_t *lineport_list;
    uint8_t *lineport_channel_list;
    uint32_t value_cnt;
    uint8_t max_port_cnt;
    uint8_t min_port;
} xconnect_configuration_t;

static int get_port_count(uint8_t port_no, struct mepa_device *dev, uint8_t *max_port_cnt, uint8_t *min_port)
{
    mepa_phy_info_t phy_info_part;
    mepa_rc rc;

    if ((rc = mepa_phy_info_get(dev, &phy_info_part)) != MEPA_RC_OK) {
        cli_printf(" Error in Getting PHY Info on port : %d\n", port_no);
        return ERROR;
    }
    switch (phy_info_part.part_number) {
    case LAN80XX_DEV_ID_8024:
    case LAN80XX_DEV_ID_8022:
    case LAN80XX_DEV_ID_8023:
    case LAN80XX_DEV_ID_8264:
        *max_port_cnt = 2;
        *min_port = 3;
        break;
    default:
        *max_port_cnt = 4;
        *min_port = 1;
        break;
    }
    return rc;
}

static void cli_cmd_xconnect_conf_get(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    phy25g_xconnect_get_conf_t conf;
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];
    uint8_t max_port_cnt = 0;
    uint8_t min_port = 0;

    if ((rc = phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        if (lan80xx_xconnect_conf_get(dev, req->port_no, &conf) != MESA_RC_OK) {
            T_E("Error in Getting xconnect failover configuration\n");
        } else {
            cli_printf("\nCrossconnect Failover configuration\n\n");
            cli_printf("Enable :	%d\n", conf.failover_conf.enable);
            cli_printf("Source Event :	%d\n", conf.failover_conf.src_event);
            if (conf.failover_conf.enable) {
                cli_printf("Mode :	%s\n", (conf.failover_conf.mode == LAN80XX_AUTO_FAILOVER_1_ISTO_1_PROTECTION) ? "1_ISTO_1_Hostprotection" : "1_PLUS_1_Hostprotection");
            } else {
                cli_printf("Mode :	%s\n", "AUTOFAILOVER_PROTECTION_NONE");
            }
            cli_printf("Role :	%s\n", (conf.failover_conf.role == LAN80XX_WPS_PHY_ROLE) ? "PHY_ROLE" : "SYSTEM_ROLE");
            cli_printf("Mac change :	%d\n", conf.failover_conf.is_mac_change);
            cli_printf("Reversion :	%d\n", conf.failover_conf.reversion);
            cli_printf("Filter enable :	%d\n", conf.failover_conf.filter_ena);
            cli_printf("Assert filter Value :	%ld\n", conf.failover_conf.assert_filter_val);
            cli_printf("Deassert filter Value :	%ld\n", conf.failover_conf.deassert_filter_val);
            cli_printf("Active Host :	%s\n", (conf.failover_conf.active_host == LAN80XX_WPS_0_1_DEFAULT_ACTIVE_H0_H3) ? "H0_H3_activehost" : "H1_H2_activehost");
            cli_printf("FC Signal Enable :	0x%d\n", conf.failover_conf.fc_signal_enable);
            cli_printf("ACK Timer :	0x%d\n", conf.failover_conf.ack_timer);
            rc = get_port_count(req->port_no, dev, &max_port_cnt, &min_port);
            if (rc != MEPA_RC_OK) {
                T_E("Error in getting port count\n");
                free(conf.Lineport_chn);
                return;
            }
            cli_printf("\nCrossconnect Anyline to Anyhost configuration\n\n");
            for (int port = 0; port < max_port_cnt; port++) {
                cli_printf("Hostport%d mapped to Lineport%d\n", port, conf.Lineport_chn[port]);
            }
            free(conf.Lineport_chn);
        }
    }
}

static void cli_cmd_xconnect_force_failover(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];

    if ((rc = phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        if (lan80xx_xconnect_force_failover(dev, req->port_no) != MESA_RC_OK) {
            T_E("Error in Xconnect force failover configuration\n");
        }
    }
}

static void cli_cmd_xconnect_hostfailover_recovery(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];

    if ((rc = phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        if (lan80xx_xconnect_failover(dev, req->port_no) != MESA_RC_OK) {
            T_E("Error in Xconnect failover recovery configuration\n");
        }
    }
}

static int cli_parm_value_port(cli_req_t *req)
{
    xconnect_configuration_t *mreq = req->module_req;
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];
    mepa_rc rc;

    rc = get_port_count(req->port_no, dev, &mreq->max_port_cnt, &mreq->min_port);
    if (rc != MEPA_RC_OK) {
        T_E("Error in getting port count\n");
        return ERROR;
    }
    mreq->lineport_list = (uint32_t *)malloc(mreq->max_port_cnt * sizeof(uint32_t));
    if (mreq->lineport_list == NULL) {
        T_E("lineport_list Allocation failed\n");
        return ERROR;
    }
    int ret = cli_parse_values(req->cmd, mreq->lineport_list, &mreq->value_cnt, mreq->min_port, 4, mreq->max_port_cnt);
    if (mreq->value_cnt != mreq->max_port_cnt) {
        T_E("Only Line ports (1,2,3,4) can be configured\n");
        free(mreq->lineport_list);
        return ERROR;
    }
    return ret;
}

static void cli_cmd_xconnect_anylinetoanyhost(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    mepa_conf_t config;
    xconnect_configuration_t *mreq = req->module_req;
    mesa_port_no_t iport, port = mreq->max_port_cnt;
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];

    if ((rc = phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        mreq->lineport_channel_list = (uint8_t *)malloc(mreq->max_port_cnt * sizeof(uint8_t));
        if (mreq->lineport_channel_list == NULL) {
            T_E("lineport_list Allocation failed\n");
            free(mreq->lineport_list);
            return;
        }
        for (iport = 0; iport < mreq->max_port_cnt; iport++) {
            if ((meba_phy_conf_get(meba_xc_phy_instance, (mreq->lineport_list[iport] - 1), &config)) != MESA_RC_OK) {
                T_E("\nMALIBU25G: Error in getting channel ID configuration for port %d\n", mreq->lineport_list[iport]);
                free(mreq->lineport_channel_list);
                free(mreq->lineport_list);
                return;
            }
            mreq->lineport_channel_list[port - 1] =  config.conf_25g.channel_id - 1;
            port--;
        }
        if (lan80xx_xconnect_anylinetoanyhost(dev, req->port_no, req->enable, mreq->lineport_channel_list) != MESA_RC_OK) {
            T_E("Error in Xconnect anyline to anyhost configuration\n");
        }
        free(mreq->lineport_channel_list);
        free(mreq->lineport_list);
    }
}

static void cli_cmd_XconnectFailover(cli_req_t *req)
{
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];
    vtss_phy_10g_auto_failover_conf_t conf;
    phy25g_autofailover_t xconnect_conf;
    demo_phy_info_t phy_family;
    struct json_object *jobj = NULL;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    int size;
    char port_xconnect_file[128];
    char *buffer;

    if (!req->set) {
        cli_printf("\n Syntax : Xconnect_failover <port_no> [-f] [filename]\n");
        T_E("\n Provide Port no and filename as arguments\n");
        return;
    }
    if (phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (dev == NULL) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    phy_data_t *data = (phy_data_t *) dev->data;
    snprintf(port_xconnect_file, sizeof(port_xconnect_file), "/root/mepa_scripts/%s", req->file_name);
    FILE *fp = fopen(port_xconnect_file, "r");
    if (fp == NULL) {
        T_E("%s : file open error file open error. Please provide Valid JSON file: xconnect_config.json\n", req->file_name);
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
        if (phy_family.family == PHY_FAMILY_MALIBU_10G) {
            if (json_rpc_get_name_json_object(&json_req, jobj, "phy_m10g", &json_req.params) != MESA_RC_OK) {
                T_E("port_xconnect object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if (json_rpc_get_vtss_phy_10g_auto_failover_conf_t(&json_req, json_req.params, &conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if (vtss_phy_10g_auto_failover_set(data->vtss_instance, &conf) != MESA_RC_OK) {
                T_E("Error in Configuring Cross connect in 10G PHY");
                goto file_close;
            }
        } else if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
            if (json_rpc_get_name_json_object(&json_req, jobj, "phy_m25g", &json_req.params) != MESA_RC_OK) {
                T_E("port_xconnect object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if (json_rpc_get_phy25g_autofailover_t(&json_req, json_req.params, &xconnect_conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if ( lan80xx_xconnect_hostfailover_Protection (dev, req->port_no, &xconnect_conf) != MESA_RC_OK) {
                T_E("Error in Configuring Cross connect in 25G PHY");
                goto file_close;
            }
        }
        cli_printf("Xconnect failover configured successfully\n");
file_close:
        free(buffer);
        fclose(fp);
    }
    return;
}

static cli_parm_t cli_parm_table[] = {

    {
        "<lineport_list>",
        "lineport_list   : List of Lineport numbers maps to Hostport 1,2,3,4 \n",
        CLI_PARM_FLAG_SET,
        cli_parm_value_port,
    }
};

static cli_cmd_t cli_cmd_table[] = {
    {
        "Xconnect_failover <port_no> [-f] [filename]",
        "Set Cross connect failover for the Particular Port",
        cli_cmd_XconnectFailover
    },
    {
        "CrossConnect_AnyLineToAnyHost <port_no> [enable|disable] <lineport_list>",
        "Configure any hostports to user defined line ports",
        cli_cmd_xconnect_anylinetoanyhost
    },
    {
        "CrossConnect_failover_recovery  <port_no>",
        "Xconnect Failover recovery for connection fault detected port",
        cli_cmd_xconnect_hostfailover_recovery
    },
    {
        "CrossConnect_force_failover  <port_no>",
        "Xconnect Force failover for particular port",
        cli_cmd_xconnect_force_failover
    },
    {
        "Xconnect_conf_get  <port_no>",
        "Get Xconnect for particular port",
        cli_cmd_xconnect_conf_get
    }
};

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

void mscc_appl_phy_xconnect(mscc_appl_init_t *init)
{
    meba_xc_phy_instance = init->board_inst;
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
