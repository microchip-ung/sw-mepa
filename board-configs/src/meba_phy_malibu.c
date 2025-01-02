// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <microchip/ethernet/board/api.h>
#include <vtss_phy_api.h>
#include "meba_aux.h"
#include "meba_generic.h"


typedef struct
{
    mepa_phy_channel_id_t    channel_id;     /* Channel Id of the PHY */
    uint16_t                 gpio_i2c_clk;   /* GPIO No of I2C Clk */
    uint16_t                 gpio_i2c_data;  /* GPIO No of I2C Data */
} edsx_phy_config_map_t;

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


/* reseting the malibu */
void m10g_mode_conf(const vtss_inst_t inst, meba_inst_t meba_inst, mepa_port_no_t iport, mepa_port_no_t slot_port)
{
    
    mesa_rc rc = MESA_RC_OK;
    vtss_gpio_10g_gpio_mode_t gpio_conf;
    const edsx_phy_config_map_t *gmap = &vsc825x_gpio_map[iport - slot_port]; /*have to be changed to respective slot */
    mepa_conf_t conf;
    conf.speed = MESA_SPEED_10G;
    conf.conf_10g.oper_mode = MEPA_PHY_LAN_MODE;
    conf.conf_10g.interface_mode = MEPA_PHY_SFI_XFI;
    conf.conf_10g.channel_id = gmap->channel_id;
    conf.conf_10g.h_media = MEPA_MEDIA_TYPE_SR;
    conf.conf_10g.l_media = MEPA_MEDIA_TYPE_SR;
    conf.conf_10g.channel_high_to_low = true;   /* Change this to "false" if Connecting the M10G PHY with increasing Channel ID */
    if(conf.conf_10g.channel_high_to_low == false ) {
        conf.conf_10g.channel_id = VTSS_CHANNEL_AUTO;
    }
    if ((rc = mepa_conf_set(meba_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
        printf("mepa_conf_set failed on port %u", iport);
	return;
    }
     /* Configure I2c Slave pins clk,data(for SFP access on line)  */
    if ((vtss_phy_10g_gpio_mode_get(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf)) == MESA_RC_OK) {
        gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_conf.p_gpio = 2;
        gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_CLK_OUT;
    
    if(vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf) != MESA_RC_OK) {
       printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
       return;
    }
    gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
    gpio_conf.p_gpio = 3;
    gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_DATA_OUT;
    if(vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_data, &gpio_conf) != MESA_RC_OK) {
        printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
	return;
    }
  } 
}
