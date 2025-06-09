// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <microchip/ethernet/board/api.h>
#include <vtss_phy_api.h>
#include "meba_aux.h"
#include "meba_generic.h"

typedef struct {
    mepa_phy_channel_id_t    channel_id;     /* Channel Id of the PHY */
    uint16_t                 gpio_i2c_clk;   /* GPIO No of I2C Clk */
    uint16_t                 gpio_i2c_data;  /* GPIO No of I2C Data */
} edsx_phy_config_map_t;

static const edsx_phy_config_map_t lan80xx_phy_map[] = {
    /* Channel 3 */
    {
        MEPA_CHANNELID_3,
        26, 27,
    },
    /* Channel 2 */
    {
        MEPA_CHANNELID_2,
        18, 19,
    },
    /* Channel 1 */
    {
        MEPA_CHANNELID_1,
        10, 11,
    },
    /* Channel 0 */
    {
        MEPA_CHANNELID_0,
        2, 3,
    },
};

static const edsx_phy_config_map_t vsc825x_gpio_map[] = {
    /* P49: CH3 */
    {
        MEPA_CHANNELID_3,
        27, 26,
    },
    /* P50: CH2 */
    {
        MEPA_CHANNELID_2,
        19, 18,
    },
    /* P51: CH1 */
    {
        MEPA_CHANNELID_1,
        11, 10,
    },
    /* P52: CH0 */
    {
        MEPA_CHANNELID_0,
        3, 2,
    },
};

void lan80xx_phy_conf(meba_inst_t inst, mepa_port_no_t port_no, mepa_port_no_t slot_port, uint8_t speed)
{
    mepa_rc rc = MEPA_RC_ERROR;
    const edsx_phy_config_map_t *map = &lan80xx_phy_map[port_no - slot_port];
    mepa_conf_t   conf = {0};
    mepa_gpio_conf_t  gpio_conf = {0};
    conf.fdx = 1;

    /* Polarity configurations in M25G EVB */
    conf.conf_25g.polarity.line_tx = 0;
    conf.conf_25g.polarity.line_rx = 0;
    conf.conf_25g.polarity.host_tx = 0;
    conf.conf_25g.polarity.host_rx = 0;

    /* Speed Config based on SKU */
    switch (speed) {
    case MEBA_PHY_PORT_MAX_25G_SPEED:
        conf.speed = MESA_SPEED_25G;
        conf.conf_25g.host_media = MEPA_MEDIA_TYPE_SFP28_25G_DAC1M;
        conf.conf_25g.line_media = MEPA_MEDIA_TYPE_SFP28_25G_SR;
        break;
    case MEBA_PHY_PORT_MAX_10G_SPEED:
        conf.speed = MESA_SPEED_10G;
        conf.conf_25g.host_media = MEPA_MEDIA_TYPE_DAC;
        conf.conf_25g.line_media = MEPA_MEDIA_TYPE_SR;
        break;
    default:
        conf.speed = MESA_SPEED_UNDEFINED;
        break;
    }
    conf.conf_25g.channel_id = map->channel_id;
    if ((rc = mepa_conf_set(inst->phy_devices[port_no], &conf)) != MESA_RC_OK) {
        printf("mepa_conf_set failed on port %u", port_no);
        return;
    }
    /* GPIO Pins are Configured to Alternate Mode to perform SFP I2C Read */
    gpio_conf.pp_enable = 0;
    gpio_conf.gpio_no = map->gpio_i2c_clk;
    gpio_conf.mode = MEPA_GPIO_MODE_ALT;
    if ((rc = mepa_gpio_mode_set(inst->phy_devices[port_no], &gpio_conf)) != MESA_RC_OK) {
        printf("mepa_gpio_conf_set for GPIO No %d failed on port %u", map->gpio_i2c_clk, port_no);
        return;
    }

    gpio_conf.gpio_no = map->gpio_i2c_data;
    gpio_conf.mode = MEPA_GPIO_MODE_ALT;
    if ((rc = mepa_gpio_mode_set(inst->phy_devices[port_no], &gpio_conf)) != MESA_RC_OK) {
        printf("mepa_conf_set for GPIO No %d failed on port %u", map->gpio_i2c_data, port_no);
        return;
    }
    return;
}


/* reseting the malibu */
void m10g_mode_conf(const vtss_inst_t inst, meba_inst_t meba_inst, mepa_port_no_t iport, mepa_port_no_t slot_port)
{

    mesa_rc rc = MESA_RC_OK;
    vtss_gpio_10g_gpio_mode_t gpio_conf;
    const edsx_phy_config_map_t *gmap = &vsc825x_gpio_map[iport - slot_port]; /*have to be changed to respective slot */
    mepa_conf_t conf = {0};

    if ((rc = mepa_conf_get(meba_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
        printf("mepa_conf_get failed on port %u", iport);
        return;
    }
    conf.speed = MESA_SPEED_10G;
    conf.conf_10g.oper_mode = MEPA_PHY_LAN_MODE;
    conf.conf_10g.interface_mode = MEPA_PHY_SFI_XFI;
    conf.conf_10g.channel_id = gmap->channel_id;
    conf.conf_10g.h_media = MEPA_MEDIA_TYPE_SR;
    conf.conf_10g.l_media = MEPA_MEDIA_TYPE_SR;
    conf.conf_10g.channel_high_to_low = true;   /* Change this to "false" if Connecting the M10G PHY with increasing Channel ID */
    if (conf.conf_10g.channel_high_to_low == false ) {
        conf.conf_10g.channel_id = VTSS_CHANNEL_AUTO;
    }
    conf.conf_10g.polarity.host_rx = false;
    conf.conf_10g.polarity.line_rx = false;
    conf.conf_10g.polarity.host_tx = false;
    conf.conf_10g.polarity.line_tx = false;
    conf.conf_10g.h_clk_src_is_high_amp = true;
    conf.conf_10g.l_clk_src_is_high_amp = true;
    if ((rc = mepa_conf_set(meba_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
        printf("mepa_conf_set failed on port %u", iport);
        return;
    }
    /* Configure I2c Slave pins clk,data(for SFP access on line)  */
    if ((vtss_phy_10g_gpio_mode_get(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf)) == MESA_RC_OK) {
        gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_conf.p_gpio = 2;
        gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_CLK_OUT;

        if (vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf) != MESA_RC_OK) {
            printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
            return;
        }
        gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_conf.p_gpio = 3;
        gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_DATA_OUT;
        if (vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_data, &gpio_conf) != MESA_RC_OK) {
            printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
            return;
        }
    }
}
