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

static void cli_cmd_XconnectFailover(cli_req_t *req)
{
    struct mepa_device *dev = meba_xc_phy_instance->phy_devices[req->port_no];
    vtss_phy_10g_auto_failover_conf_t conf;
    demo_phy_info_t phy_family;
    struct json_object *jobj = NULL;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    int size;
    char port_xconnect_file[128];
    char *buffer;

    if(!req->set) {
        cli_printf("\n Syntax : Xconnect_failover <port_no> [-f] [filename]\n");
        T_E("\n Provide Port no and filename as arguments\n");
        return;
    }
    if (phy_family_detect(meba_xc_phy_instance, req->port_no, &phy_family) != MEPA_RC_OK)     {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if(dev == NULL) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    phy_data_t *data = (phy_data_t *) dev->data;
    snprintf(port_xconnect_file, sizeof(port_xconnect_file), "/root/mepa_scripts/%s", req->file_name);
    FILE *fp = fopen(port_xconnect_file, "r");
    if(fp == NULL) {
        T_E("%s : file open error file open error. Please provide Valid JSON file: xconnect_config.json\n", req->file_name);
        return;
    }
    if(!(fseek(fp, 0, SEEK_END)) && ftell(fp)) {
        size = ftell(fp);
        buffer = (char*)malloc(size);
        if(buffer == NULL) {
            T_E("Fatal: failed to allocate %d bytes.\n", size);
            goto file_close;
        }
        fseek(fp, 0, SEEK_SET);
        if(!fread(buffer, 1, size, fp)) {
            T_E("File read error");
            goto file_close;
        }
        jobj = json_tokener_parse(buffer);
        if(jobj == NULL) {
            T_E("could not parse parms from file: %s", req->file_name);
            goto file_close;
        }
        if(phy_family.family == PHY_FAMILY_MALIBU_10G)
        {
            if(json_rpc_get_name_json_object(&json_req, jobj, "phy_m10g", &json_req.params) != MESA_RC_OK) {
                T_E("port_xconnect object name not found in file: %s", req->file_name);
                goto file_close;
            }
            if(json_rpc_get_vtss_phy_10g_auto_failover_conf_t(&json_req, json_req.params, &conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if(vtss_phy_10g_auto_failover_set(data->vtss_instance, &conf) != MESA_RC_OK) {
                T_E("Error in Configuring Cross connect in 10G PHY");
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

static cli_cmd_t cli_cmd_table[] = {
    {
        "Xconnect_failover <port_no> [-f] [filename]",
        "Set Cross connect failover for the Particular Port",
        cli_cmd_XconnectFailover
    },
};

static void phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++)
    {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
}

void mscc_appl_phy_xconnect(mscc_appl_init_t *init)
{
    meba_xc_phy_instance = init->board_inst;
    switch (init->cmd)
    {
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
