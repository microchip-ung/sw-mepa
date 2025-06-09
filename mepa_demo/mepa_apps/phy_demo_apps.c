// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "cli.h"
#include "phy_demo_apps.h"


mepa_rc phy_family_detect(meba_inst_t meba_instance, mepa_port_no_t port_no, demo_phy_info_t *phy_info)
{
    mepa_rc rc;
    mepa_phy_info_t phy_info_part;
    if ((rc = mepa_phy_info_get(meba_instance->phy_devices[port_no], &phy_info_part)) != MEPA_RC_OK) {
        cli_printf(" Error in Getting PHY Info on port : %d\n", port_no);
        return MEPA_RC_ERROR;
    }
    switch (phy_info_part.part_number) {
    case PHY_TYPE_8021:
    case PHY_TYPE_8022:
    case PHY_TYPE_8041:
    case PHY_TYPE_8042:
    case PHY_TYPE_8043:
    case PHY_TYPE_8044:
        phy_info->family = PHY_FAMILY_MALIBU_25G;
        break;
    case PHY_TYPE_8256:
    case PHY_TYPE_8257:
    case PHY_TYPE_8258:
        phy_info->family = PHY_FAMILY_MALIBU_10G;
        break;
    case PHY_TYPE_8582:
    case PHY_TYPE_8584:
    case PHY_TYPE_8575:
    case PHY_TYPE_8564:
    case PHY_TYPE_8562:
    case PHY_TYPE_8586:
        phy_info->family = PHY_FAMILY_VIPER;
        break;
    case PHY_TYPE_8574:
    case PHY_TYPE_8504:
    case PHY_TYPE_8572:
    case PHY_TYPE_8552:
        phy_info->family = PHY_FAMILY_TESLA;
        break;
    case PHY_TYPE_8814:
        phy_info->family = PHY_FAMILY_LAN8814;
        break;
    default:
        cli_printf("\nPHY Connected is not supported in application\n");
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}


mepa_rc mepa_dev_create_check(meba_inst_t meba_instance, mepa_port_no_t port_no)
{
    if (!meba_instance->phy_devices[port_no]) {
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}

int atoi_Conversion(const char *strg)
{

    // Initialize res to 0
    int res = 0;
    int i = 0;

    // Iterate through the string strg and compute res
    while (strg[i] != '\0') {
        res = res * 10 + (strg[i] - '0');
        i++;
    }
    return res;
}
