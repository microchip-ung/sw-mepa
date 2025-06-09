// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "microchip/ethernet/phy/api/types.h"

#include "vtss_phy_10g_api.h"
#include "vtss_private.h"

#include "main.h"
#include "mesa-rpc.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "mepa_driver.h"
#include "phy_demo_apps.h"

static void cli_cmd_cable_diagnostics (cli_req_t *req);

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_diagnostics"
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

static meba_inst_t meba_phy_diag_inst;

static cli_cmd_t cli_cmd_table[] = {
    {
        "cable_diagnostics <port_no>",
        "start cable diagnostics for particular port number of phy",
        cli_cmd_cable_diagnostics
    }

};

static void cli_cmd_cable_diagnostics(cli_req_t *req)
{
    mepa_cable_diag_result_t result = {};
    mepa_conf_t conf;
    int mode = 0;
    mepa_rc rc;
    demo_phy_info_t phy_family;
    char *CableDiagStatusStrings[] = {"OK", "OPEN", "SHORT", NULL, "ABNORM"};
    char *PairNameStrings[] = {"A", "B", "C", "D"};

    if ((rc = phy_family_detect(meba_phy_diag_inst, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if (phy_family.family == PHY_FAMILY_MALIBU_10G) {
        T_E("\n PHY on Port :%d doesn't support Cable Diagnostics \n", req->port_no);
        return;
    }
    if (mepa_conf_get(meba_phy_diag_inst->phy_devices[req->port_no], &conf) != MEPA_RC_OK) {
        T_E("\n Error in Getting conf set of phy for port :%d \n", req->port_no);
        return;
    }
    if (conf.admin.enable) {
        if (conf.speed == MESA_SPEED_AUTO || conf.speed == MESA_SPEED_1G) {
            mode = 0; //ANEG Mode
        } else {
            mode = 1; // Forced Mode
        }
    } else {
        mode = 2; // Power_Down Mode
    }
    if (mepa_cable_diag_start(meba_phy_diag_inst->phy_devices[req->port_no], mode) != MEPA_RC_OK) {
        T_E("\n Error in Starting cable diag on Port :%d \n", req->port_no);
        return;
    }
    /*This wait timer is for completion of Cable Diagnostics STATUS - OK or ERROR*/
    while (1) {
        rc = mepa_cable_diag_get(meba_phy_diag_inst->phy_devices[req->port_no], &result);
        MEPA_MSLEEP(5);
        if (rc == MEPA_RC_OK || rc == MEPA_RC_ERROR) {
            break;
        }
    }
    if (rc == MEPA_RC_ERROR) {
        T_E("\n Timeout occured error in reading cable diagnostic result :%d \n", req->port_no);
        return;
    }
    if (result.link) {
        cli_printf("\n Cable Diagnostics cannot be performed for the Port %d when link is UP \n", req->port_no);
        return;
    }
    cli_printf("\nCable Diagnostics Result on Port %d\n", req->port_no);
    cli_printf("\n%-6s %-6s %-10s %-6s\n", "Link", "Pair", "Status", "Length");

    for (int i = 0; i < 4; i++) {
        cli_printf("%-6d %-6s %-10s %-6d\n",
                   result.link,
                   PairNameStrings[i],
                   CableDiagStatusStrings[result.status[i]],
                   result.length[i]);
    }
    return;
}

static void phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table) / sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

}

void mscc_appl_phy_diagnostics_demo(mscc_appl_init_t *init)
{
    meba_phy_diag_inst = init->board_inst;
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


