// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "phy_demo_apps.h"
#include "phy_gpio_demo.h"
#include <vtss_phy_api.h>
#include "lan80xx.h"

#define LAN80XX_MAX_GPIO_NO 40
#define LAN80XX_TOTAL_MAX_EVENTS 127
#define LAN80XX_MAX_EVENTS 63
#define ERROR -1

meba_inst_t meba_gpio_lp_instance;

static mscc_appl_trace_module_t trace_module = {
    .name = "gpio_lp_app"
};

enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};

mepa_bool_t event_poll_en[MAX_PORTS];
typedef struct {
    uint32_t event_list[LAN80XX_TOTAL_MAX_EVENTS];
    uint32_t event_cnt;
    mepa_bool_t intr_a;
} event_configuration_t;

char *event_list[] = {
    "LINE_PCS1G_LINK_DOWN_INTR",
    "LINE_PCS1G_OUT_OF_SYNC_INTR",
    "HOST_PCS1G_LINK_DOWN_INTR",
    "HOST_PCS1G_OUT_OF_SYNC_INTR",
    "LINE_PCS25G_ALIGN_DONE_INTR",
    "LINE_PCS25G_BLOCK_LOCK_INTR",
    "LINE_PCS25G_HI_BER_INTR",
    "HOST_PCS25G_ALIGN_DONE_INTR",
    "HOST_PCS25G_BLOCK_LOCK_INTR",
    "HOST_PCS25G_HI_BER_INTR",
    "H2L_RA_FIFO_UNDERFLOW_INTR",
    "H2L_RA_FIFO_OVERFLOW_INTR",
    "L2H_RA_FIFO_UNDERFLOW_INTR",
    "L2H_RA_FIFO_OVERFLOW_INTR",
    "XOFF_PAUSE_GEN_INTR",
    "XON_PAUSE_GEN_INTR",
    "TX_UNCORRECTED_FRM_DROP_INTR",
    "RX_UNCORRECTED_FRM_DROP_INTR",
    "TX_CTRL_QUEUE_OVERFLOW_DROP_INTR",
    "TX_CTRL_QUEUE_UNDERFLOW_DROP_INTR",
    "TX_DATA_QUEUE_OVERFLOW_DROP_INTR",
    "TX_DATA_QUEUE_UNDERFLOW_DROP_INTR",
    "RX_OVERFLOW_DROP_INTR",
    "RX_UNDERFLOW_DROP_INTR",
    "INGR_ECC_INTERRUPT_STATUS_INTR",
    "EGR_ECC_INTERRUPT_STATUS_INTR",
    "INGR_FC_BUFFER_INIT_DONE_INTR",
    "EGR_FC_BUFFER_INIT_DONE_INTR",
    "INGR_FC_BUFFER_SEC_INTR",
    "INGR_FC_BUFFER_DED_INTR",
    "INGR_FC_BUFFER_FAULT_READ_INTR",
    "EGR_FC_BUFFER_SEC_INTR",
    "EGR_FC_BUFFER_DED_INTR",
    "EGR_FC_BUFFER_FAULT_READ",
    "HOST_MAC_TX_ABORT_INTR",
    "HOST_MAC_TX_UNDERFLOW_INTR",
    "HOST_MAC_RX_TAG_INTR",
    "HOST_MAC_RX_MPLS_UC_INTR",
    "HOST_MAC_RX_MPLS_MC_INTR",
    "HOST_MAC_RX_NON_STD_PREAMBLE_INTR",
    "HOST_MAC_RX_PREAM_ERR_INTR",
    "HOST_MAC_RX_PREAMBLE_MISMATCH_INTR",
    "HOST_MAC_RX_PREAMBLE_SHRINK_INTR",
    "HOST_MAC_RX_IPG_SHRINK_INTR",
    "LINE_MAC_TX_ABORT_INTR",
    "LINE_MAC_TX_UNDERFLOW_INTR",
    "LINE_MAC_RX_TAG_INTR",
    "LINE_MAC_RX_MPLS_UC_INTR",
    "LINE_MAC_RX_MPLS_MC_INTR",
    "LINE_MAC_RX_NON_STD_PREAMBLE_INTR",
    "LINE_MAC_RX_PREAM_ERR_INTR",
    "LINE_MAC_RX_PREAMBLE_MISMATCH_INTR",
    "LINE_MAC_RX_PREAMBLE_SHRINK_INTR",
    "LINE_MAC_RX_IPG_SHRINK_INTR",
    "HOST_MAC_DIS_STATE_INTR",
    "HOST_MAC_IDLE_STATE_INTR",
    "HOST_MAC_REMOTE_ERR_STATE_INTR",
    "HOST_MAC_LOCAL_ERR_STATE_INTR",
    "LINE_MAC_DIS_STATE_INTR",
    "LINE_MAC_IDLE_STATE_INTR",
    "LINE_MAC_REMOTE_ERR_STATE_INTR",
    "LINE_MAC_LOCAL_ERR_STATE_INTR",
    "GPIO_INTR_0",
    "GPIO_INTR_1",
};

char *ext_event_list[] = {
    "HOST_PMAC_TX_ABORT_INTR",
    "HOST_PMAC_TX_UNDERFLOW_INTR",
    "HOST_PMAC_RX_MPLS_UC_INTR",
    "HOST_PMAC_RX_MPLS_MC_INTR",
    "HOST_PMAC_RX_NON_STD_PREAMBLE_INTR",
    "HOST_PMAC_RX_PREAMBLE_ERR_INTR",
    "HOST_PMAC_RX_PREAMBLE_MISMATCH_INTR",
    "HOST_PMAC_RX_PREAMBLE_SHRINK_INTR",
    "LINE_PMAC_TX_ABORT_INTR",
    "LINE_PMAC_TX_UNDERFLOW_INTR",
    "LINE_PMAC_RX_MPLS_UC_INTR",
    "LINE_PMAC_RX_MPLS_MC_INTR",
    "LINE_PMAC_RX_NON_STD_PREAMBLE_INTR",
    "LINE_PMAC_RX_PREAMBLE_ERR_INTR",
    "LINE_PMAC_RX_PREAMBLE_MISMATCH_INTR",
    "LINE_PMAC_RX_PREAMBLE_SHRINK",
    "HOST_MAC_MM_PRMPT_ACTIVE_INTR",
    "HOST_MAC_MM_PRMPT_UNEXP_RX_PFRM_INTR",
    "HOST_MAC_MM_PRMPT_UNEXP_TX_PFRM_INTR",
    "LINE_MAC_MM_PRMPT_ACTIVE_INTR",
    "LINE_MAC_MM_PRMPT_UNEXP_RX_PFRM_INTR",
    "LINE_MAC_MM_PRMPT_UNEXP_TX_PFRM_INTR",
    "HOST_PMA_RESET_DONE_INTR",
    "HOST_PMA_RXEI_FILTERED_INTR",
    "LINE_PMA_RESET_DONE_INTR",
    "LINE_PMA_RXEI_FILTERED_INTR",
    "WPS1_FC_ACK_TIMER_INTR",
    "WPS0_FC_ACK_TIMER_INTR",
    "WPS1_CONN_FAULT_INTR",
    "WPS0_CONN_FAULT_INTR",
    "WPS1_FAILOVER_INTR",
    "WPS0_FAILOVER_INTR",
    "H0_SWITCH_INTR",
    "H1_SWITCH_INTR",
    "H2_SWITCH_INTR",
    "H3_SWITCH_INTR",
    "L0_SWITCH_INTR",
    "L1_SWITCH_INTR",
    "L2_SWITCH_INTR",
    "L3_SWITCH_INTR",
    "H0_COND_ALT_DET_INTR",
    "H1_COND_ALT_DET_INTR",
    "H2_COND_ALT_DET_INTR",
    "H3_COND_ALT_DET_INTR",
    "L0_COND_ALT_DET_INTR",
    "L1_COND_ALT_DET_INTR",
    "L2_COND_ALT_DET_INTR",
    "L3_COND_ALT_DET_INTR",
    "H0_COND_ALT_UNF_DET_INTR",
    "H1_COND_ALT_UNF_DET_INTR",
    "H2_COND_ALT_UNF_DET_INTR",
    "H3_COND_ALT_UNF_DET_INTR",
    "L0_COND_ALT_UNF_DET_INTR",
    "L1_COND_ALT_UNF_DET_INTR",
    "L2_COND_ALT_UNF_DET_INTR",
    "L3_COND_ALT_UNF_DET_INTR",
    "H0_FIFO_ERROR_INTR",
    "H1_FIFO_ERROR_INTR",
    "H2_FIFO_ERROR_INTR",
    "H3_FIFO_ERROR_INTR",
    "L0_FIFO_ERROR_INTR",
    "L1_FIFO_ERROR_INTR",
    "L2_FIFO_ERROR_INTR",
    "L3_FIFO_ERROR_INTR",
};

static mscc_appl_trace_group_t trace_groups[10] = {
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

char *gpio_txt[] = {"Output", "Input", "Alternate"};

static int phy_gpio_no_for_alternate_function(phy_family_t family, int alt_fun, gpio_table_t *gpio_table)
{
    if (family == PHY_FAMILY_VIPER) {
        memcpy(gpio_table, &viper[alt_fun], sizeof(gpio_table_t));
        return viper[alt_fun].gpio_no;
    }
    if (family == PHY_FAMILY_LAN8814) {
        memcpy(gpio_table, &lan8814[alt_fun], sizeof(gpio_table_t));
        return lan8814[alt_fun].gpio_no;
    }
    return 0;
}

static void cli_cmd_gpio_conf(cli_req_t *req)
{
    mepa_rc rc;
    mepa_gpio_conf_t gpio_conf;
    demo_phy_info_t phy_family;
    int gpio_mode = 0, alt_mode = 0, led_number = 0, alt_fun = 0, gpio_intr = 0;
    uint8_t gpio_num = 0, max_alter_fun = 0;
    int vsc_phy_connected = 0;
    gpio_table_t gpio_table = {0};

    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    if ((rc = phy_family_detect(meba_gpio_lp_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    T_I("\n Detected PHY Family on port %d is : %d\n", req->port_no, phy_family.family);
    if ((phy_family.family == PHY_FAMILY_VIPER) || (phy_family.family == PHY_FAMILY_TESLA) || (phy_family.family == PHY_FAMILY_LAN8814)) {
        memset(&gpio_conf, 0, sizeof(mepa_gpio_conf_t));
        cli_printf("\n");
        cli_printf("\t 1 . Output Mode\n");
        cli_printf("\t 2 . Input Mode\n");
        cli_printf("\t 3 . Alternate Mode\n");

        /* In case of Viper and Tesla PHY the Port LED Confguration is not the alternate
         * function of GPIO PIN, it has seperate Register for LED Configuration, provided
         * sperate Mode to configure LED in case of Viper and Tesla PHYs
         */
        if (phy_family.family != PHY_FAMILY_LAN8814) {
            cli_printf("\t 4 . LED Configuration\n");
            vsc_phy_connected = 1;
        }

        cli_printf("\t Enter the Mode of GPIO Pin : ");
        scanf("%d", &gpio_mode);

        if (gpio_mode == 1) {
            gpio_conf.mode = MEPA_GPIO_MODE_OUT;
        } else if (gpio_mode == 2) {
            gpio_conf.mode = MEPA_GPIO_MODE_IN;
        } else if (gpio_mode == 3) {
            gpio_conf.mode = MEPA_GPIO_MODE_ALT;
        } else if ((gpio_mode == 4) && vsc_phy_connected) {
            gpio_conf.mode = MEPA_GPIO_MODE_ALT;
        } else {
            T_E("\n Invalid Mode Selected \n");
            return;
        }

        /* Only for Input and Output Mode GPIO Number is required from User for Alternate Function
         * based on the functionality slected by User GPIO PIN is Automatically Assigned */
        if (gpio_conf.mode  == MEPA_GPIO_MODE_OUT || gpio_conf.mode  == MEPA_GPIO_MODE_IN) {
            cli_printf("\n\t Enter GPIO Number to be configured as %s mode: ", gpio_txt[gpio_mode - 1]);
            scanf("%hhu", &gpio_num);
            gpio_conf.gpio_no = gpio_num;
        }

        if (gpio_mode == 3) {
            cli_printf("\n");
            switch (phy_family.family) {
            case PHY_FAMILY_VIPER:
                cli_printf("\t 0 . Signal Detect 0\n");
                cli_printf("\t 1 . Signal Detect 1\n");
                cli_printf("\t 2 . Signal Detect 2\n");
                cli_printf("\t 3 . Siganl Detect 3\n");
                cli_printf("\t 4 . I2C SCL 0\n");
                cli_printf("\t 5 . I2C SCL 1\n");
                cli_printf("\t 6 . I2C SCL 2\n");
                cli_printf("\t 7 . I2C SCL 3\n");
                cli_printf("\t 8 . I2C SDA\n");
                cli_printf("\t 9 . Fast Link Fail\n");
                cli_printf("\t 10. 1588 load Save\n");
                cli_printf("\t 11. 1588 PPS 0\n");
                cli_printf("\t 12. 1588 SPI CS\n");
                cli_printf("\t 13. 1588 SPI DO\n");
                max_alter_fun = MAX_SUPPORTED_ALT_FUN_VIPER;
                break;
            case PHY_FAMILY_LAN8814:
                cli_printf("\t 0 . 1588 Event A\n");
                cli_printf("\t 1 . 1588 Event B\n");
                cli_printf("\t 2 . 1588 Ref Clk\n");
                cli_printf("\t 3 . 1588 LD Adj\n");
                cli_printf("\t 4 . 1588 STI CS\n");
                cli_printf("\t 5 . 1588 STI CLK\n");
                cli_printf("\t 6 . 1588 STI DO\n");
                cli_printf("\t 7 . RCVRD CLK IN 1\n");
                cli_printf("\t 8 . RCVRD CLK IN 2\n");
                cli_printf("\t 9 . RCVRD CLK OUT 1\n");
                cli_printf("\t 10. RCVRD CLK OUT 2\n");
                cli_printf("\t 11. SOF 0\n");
                cli_printf("\t 12. SOF 1\n");
                cli_printf("\t 13. SOF 2\n");
                cli_printf("\t 14. SOF 3\n");
                cli_printf("\t 15. Port LED Configuration\n");
                max_alter_fun = MAX_SUPPORTED_ALT_FUN_LAN8814;
                break;
            default:
                break;
            }

            cli_printf("\n\t Select Alternte Function of GPIO : ");
            scanf("%d", &alt_fun);
            if (alt_fun >=  max_alter_fun) {
                T_E("\n Alternate Function Not Supported \n");
                return;
            }
            gpio_conf.gpio_no = phy_gpio_no_for_alternate_function(phy_family.family, alt_fun, &gpio_table);
            cli_printf("\n\t Selected %s Alternate Mode for GPIO %d PIN\n", gpio_table.desc, gpio_conf.gpio_no);
        }
        /* In Case of LAN8814 PHY, Port LED alternate function the GPIO Number is selected inside the API based
         * on the Channel id of the PHY */
        if (gpio_mode == 4 || (!vsc_phy_connected && (alt_fun == 15))) {
            cli_printf("\n");
            cli_printf("\t 1. LED Link Activity, any speed link\n");
            cli_printf("\t 2. LED 1000 Mbps Link Activity\n");
            cli_printf("\t 3. LED 100 Mbps Link Activity\n");
            cli_printf("\t 4. LED 10 Mbps Link Activity\n");
            cli_printf("\t 5. LED 100/1000 Mbps Link Activity\n");
            cli_printf("\t 6. LED 10/1000 Mbps Link Activity\n");
            cli_printf("\t 7. LED 100Base-Fx/1000Base-X Link Activity\n");
            cli_printf("\t 8. LED Full Duplex and Collison Detect\n");
            cli_printf("\t 9. LED Collision Detect\n");
            cli_printf("\t 10.LED Tx/Rx Activity\n");
            cli_printf("\t 11.LED 100Base-Fx/1000Base-X Fiber Activity Detect\n");
            cli_printf("\t 12.LED ANEG Fault/Paralled Detect Fault/Paralled\n");
            cli_printf("\t 13.LED 1000 Base-X Link Activity\n");
            cli_printf("\t 14.LED 100 Base-Fx Link Activity\n");
            cli_printf("\t 15.LED 1000 base Activity\n");
            cli_printf("\t 16.LED 100 Base-Fx activity\n");
            cli_printf("\t 17.Force LED OFF\n");
            cli_printf("\t 18.Force LED On\n");
            cli_printf("\t 19.LED Fast link Fail\n");
            cli_printf("\t 20.LED Link Tx\n");
            cli_printf("\t 21.LED Link Rx\n");
            cli_printf("\t 22.LED Link Fault\n");
            cli_printf("\t 23.LED Link No Activity and Any Speed\n");
            cli_printf("\t 24.LED local Rx Error Status\n");
            cli_printf("\t 25.LED remote Rx Error Status\n");
            cli_printf("\t 26.LED Negotiated Speed\n");
            cli_printf("\t 27.LED Master Slave Mode\n");
            cli_printf("\t 28.LED PCS Tx Error Status\n");
            cli_printf("\t 29.LED PCS Rx Error Status\n");
            cli_printf("\t 30.LED PCS Tx Activity\n");
            cli_printf("\t 31.LED PCS Rx Activity\n");
            cli_printf("\t 32.LED Wake on LAN\n");
            cli_printf("\n\t Select One LED Configuration : ");
            scanf("%d", &alt_mode);
            if (alt_mode == 0 || alt_mode > 32) {
                T_E("\n Enter Valid LED Configuration \n");
                return;
            }
            gpio_conf.mode = MEPA_GPIO_MODE_ALT + alt_mode; /* Alternate Mode Selected */
            cli_printf("\n");
            if (phy_family.family != PHY_FAMILY_LAN8814) {
                cli_printf("\t Enter the LED needs to be configured [0 - LED0 or 1 - LED1] : ");
            } else {
                cli_printf("\t Enter the LED needs to be configured [0 - LED1 or 1 - LED2] : ");
            }
            scanf("%d", &led_number);
            gpio_conf.led_num = (led_number == 1) ? 1 : 0;
        }


        if ((rc = mepa_gpio_mode_set(meba_gpio_lp_instance->phy_devices[req->port_no], &gpio_conf)) != MEPA_RC_OK) {
            cli_printf(" Error in configuring GPIO : %d\n", req->port_no);
            return;
        }
    } else if (phy_family.family == PHY_FAMILY_MALIBU_10G) {
        uint16_t          gpio_no = 0;
        vtss_gpio_10g_gpio_mode_t   gpio_mode;
        memset(&gpio_mode, 0, sizeof(vtss_gpio_10g_gpio_mode_t));
        int gpio_output_mode = 0, gpio_out_signal = 0, gpio_alt = 0, gpio_aggr = 0, p_gpio = 0, gpio_conf = 0;
        uint8_t channel_id;
        char input[3];
        mepa_bool_t event_ena_dis = 0;
        gpio_table_t gpio_map;
        int event_type = 0, enable = 0;


        vtss_phy_10g_event_t       ev;
        vtss_phy_10g_extnd_event_t ext_ev;
        u64                        ex2_ev;
        vtss_phy_ts_event_t ts_ev;
        mepa_macsec_event_t event;

        cli_printf("\n");
        cli_printf("\t 1 . Output Mode\n");
        cli_printf("\t 2 . Input Mode\n");
        cli_printf("\t 3 . Alternate Mode\n");
        cli_printf("\n\t ==Enter the Mode of GPIO Pin : ");
        scanf("%s", &input[0]);
        gpio_conf = atoi_Conversion(input);
        memset(input, '\0', sizeof(input));
        if (gpio_conf == 1) {
            cli_printf("\n\t ==Enter GPIO Number : ");
            scanf("%s", &input[0]);
            gpio_no = atoi_Conversion(input);
            memset(input, '\0', sizeof(input));
            cli_printf("\n");
            cli_printf("\t 1 . No Routing only as Output PIN\n");
            cli_printf("\t 2 . Route signal to Output PIN\n");
            cli_printf("\n\t ==Enter usage of GPIO Output Pin : ");
            scanf("%s", &input[0]);
            gpio_output_mode = atoi_Conversion(input);
            memset(input, '\0', sizeof(input));

            switch (gpio_output_mode) {
            case 1:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_NONE;
                gpio_mode.invert_output = FALSE;
                break;
            case 2:
                cli_printf("\n");
                cli_printf("\t 1  . Host Link Status\n");
                cli_printf("\t 2  . Line Link Status\n");
                cli_printf("\t 3  . KR 8b10b\n");
                cli_printf("\t 4  . KR 10b\n");
                cli_printf("\t 5  . ROSI Pulse\n");
                cli_printf("\t 6  . ROSI Sdata\n");
                cli_printf("\t 7  . ROSI Sclk\n");
                cli_printf("\t 8  . TOSI Pulse\n");
                cli_printf("\t 9  . TOSI Sclk\n");
                cli_printf("\t 10 . Line PCS 1G Link\n");
                cli_printf("\n\t ==Enter Signal needs to be routed :");
                scanf("%s", &input[0]);
                gpio_out_signal = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                cli_printf("\n\t ==Enter Virtual GPIO Number p_gpio [0 - 7] for each channel: ");
                scanf("%s", &input[0]);
                p_gpio = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                if (p_gpio < 8) {
                    gpio_mode.p_gpio = p_gpio;
                } else {
                    T_E("\n Invalid Virtual GPIO Number supported 0 to 7 \n");
                    return;
                }
                gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                switch (gpio_out_signal) {
                case 1:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_HOST_LINK;
                    break;
                case 2:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LINE_LINK;
                    break;
                case 3:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LINE_KR_8b10b_2GPIO;
                    break;
                case 4:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LINE_KR_10b_2GPIO;
                    break;
                case 5:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_ROSI_PULSE;
                    break;
                case 6:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_ROSI_SDATA;
                    break;
                case 7:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_ROSI_SCLK;
                    break;
                case 8:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_TOSI_PULSE;
                    break;
                case 9:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_TOSI_SCLK;
                    break;
                case 10:
                    gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LINE_PCS1G_LINK;
                    break;
                default:
                    T_E("\n Invalid Input \n");
                    break;
                }
                break;
            default:
                T_E("\n Invalid Input \n");
                return;
            }
        } else if (gpio_conf == 2) {
            cli_printf("\n\t ==Enter GPIO Number : ");
            scanf("%s", &input[0]);
            gpio_no = atoi_Conversion(input);
            memset(input, '\0', sizeof(input));
            gpio_mode.mode  = VTSS_10G_PHY_GPIO_IN;
            gpio_mode.input = VTSS_10G_GPIO_INPUT_NONE;
            gpio_mode.p_gpio_intrpt = FALSE;
            // GPIO Input Configuration
        } else if (gpio_conf == 3) {
            cli_printf("\n");
            cli_printf("\t 1  . Rate Select\n");
            cli_printf("\t 2  . Module Absent\n");
            cli_printf("\t 3  . I2C Master Clock\n");
            cli_printf("\t 4  . I2C Master Data\n");
            cli_printf("\t 5  . Tx Disable\n");
            cli_printf("\t 6  . Tx Fault\n");
            cli_printf("\t 7  . Rx LOS\n");
            cli_printf("\t 8  . Line Link Up\n");
            cli_printf("\t 9  . LED Activity\n");
            cli_printf("\t 10 . Aggregate Interrupt\n");
            cli_printf("\n\t ==Enter The Alternate Mode to be Configured : ");
            scanf("%s", &input[0]);
            gpio_alt = atoi_Conversion(input);
            memset(input, '\0', sizeof(input));
            if (gpio_alt == 0 || gpio_alt > 10) {
                T_E("\n Invalid Input\n");
                return;
            }
            gpio_map = malibu10g_ch_0[gpio_alt - 1];

            vtss_phy_10g_channel_id_get(NULL, req->port_no, &channel_id);
            gpio_no =  (channel_id * 8) + gpio_map.gpio_no;

            switch (gpio_alt) {
            case 1:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_NONE;
                gpio_mode.invert_output = FALSE;
                break;
            case 2:
                gpio_mode.mode  = VTSS_10G_PHY_GPIO_IN;
                gpio_mode.input = VTSS_10G_GPIO_INPUT_SFP_MOD_DET;
                gpio_mode.use_as_intrpt = TRUE;
                break;
            case 3:
                gpio_mode.mode  = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_CLK_OUT;
                cli_printf("\n\t ==Enter Virtual GPIO Number p_gpio [0 - 7] for each channel : ");
                scanf("%s", &input[0]);
                p_gpio = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                if (p_gpio < 8) {
                    gpio_mode.p_gpio = p_gpio;
                } else {
                    T_E("\n Invalid Virtual GPIO Number supported 0 to 7 \n");
                    return;
                }
                gpio_mode.invert_output = 0;
                break;
            case 4:
                gpio_mode.mode   = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_DATA_OUT;
                cli_printf("\n\t ==Enter Virtual GPIO Number p_gpio [0 - 7] for each channel : ");
                scanf("%s", &input[0]);
                p_gpio = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                if (p_gpio < 8) {
                    gpio_mode.p_gpio = p_gpio;
                } else {
                    T_E("\n Invalid Virtual GPIO Number supported 0 to 7 \n");
                    return;
                }
                gpio_mode.invert_output = 0;
                break;
            case 5:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_NONE;
                break;
            case 6:
            case 7:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_IN;
                gpio_mode.input = VTSS_10G_GPIO_INPUT_NONE;
                gpio_mode.p_gpio_intrpt = 0;
                break;
            case 8:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LINE_LINK;
                cli_printf("\n\t ==Enter Virtual GPIO Number p_gpio [0 - 7] for each channel : ");
                scanf("%s", &input[0]);
                p_gpio = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                if (p_gpio < 8) {
                    gpio_mode.p_gpio = p_gpio;
                } else {
                    T_E("\n Invalid Virtual GPIO Number supported 0 to 7 \n");
                    return;
                }
                gpio_mode.invert_output = 0;
                break;
            case 9:
                gpio_mode.mode = VTSS_10G_PHY_GPIO_LED;
                gpio_mode.in_sig = VTSS_10G_GPIO_INTR_SGNL_LED_TX;
                gpio_mode.led_conf.mode = VTSS_10G_GPIO_LED_TX_LINK_TX_RX_DATA;
                gpio_mode.led_conf.blink = VTSS_10G_GPIO_LED_BLINK_NONE;
                gpio_mode.invert_output = 0;
                gpio_no = 36 + channel_id;
                // LED Activity
                break;
            case 10:
                // None
                break;
            default:
                T_E("\n Invalid Input \n");
                return;
            }
            if (gpio_alt == 10) {
                cli_printf("\n");
                cli_printf("\t 1  . WIS 0\n");
                cli_printf("\t 2  . WIS 1\n");
                cli_printf("\t 3  . Line PCS10G\n");
                cli_printf("\t 4  . Host PCS10G\n");
                cli_printf("\t 5  . Line PCS 1G\n");
                cli_printf("\t 6  . Host PCS 1G\n");
                cli_printf("\t 7  . MACsec Egress \n");
                cli_printf("\t 8  . MACsec Ingress\n");
                cli_printf("\t 9  . Line MAC\n");
                cli_printf("\t 10 . HOST MAC\n");
                cli_printf("\t 11 . FC Buffer\n");
                cli_printf("\t 12 . Line Ingess FIFO Interrupt\n");
                cli_printf("\t 13 . Line Egress FIFO Interrupt\n");
                cli_printf("\t 14 . Host Egress FIFO Interrupt\n");
                cli_printf("\t 15 . Line PMA Interrupt\n");
                cli_printf("\t 16 . Host PMA Interrupt\n");
                cli_printf("\t 17 . 1588 Interrupt\n");
                cli_printf("\t 18 . LCPLL 0 Interrupt\n");
                cli_printf("\t 19 . LCPLL 1 Interrupt\n");
                cli_printf("\t 20 . Cross connect interrupt\n");
                cli_printf("\n\t ==Enter The Interrupt to be routed to Aggr Intrpt : ");
                scanf("%s", &input[0]);
                gpio_aggr = atoi_Conversion(input);
                memset(input, '\0', sizeof(input));
                gpio_mode.mode = VTSS_10G_PHY_GPIO_AGG_INT_0;
                gpio_mode.c_intrpt = gpio_aggr - 1;
                gpio_no = 34;
                switch (gpio_aggr) {
                case 1:
                case 2:
                    gpio_mode.aggr_intrpt = 1 << (channel_id * 2);
                    event_type = MALIBU10G_EVENT;
                    ev = VTSS_PHY_10G_LINK_LOS_EV;
                    break;
                case 3:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    ext_ev = VTSS_PHY_10G_HIGHBER_EV | VTSS_PHY_10G_RX_LINK_STAT_EV;
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    break;
                case 4:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    ex2_ev = (1 << VTSS_PHY_HOST_10G_HIGHBER_EV) | (1 << VTSS_PHY_HOST_10G_RX_LINK_STAT_EV);
                    event_type = MALIBU10G_EXTENDED2_EVENT;
                    break;
                case 5:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    ex2_ev = (1 << VTSS_PHY_LINE_1G_XGMII_MASK_LINK_DOWN_MASK) | (1 << VTSS_PHY_LINE_1G_XGMII_MASK_OUT_OF_SYNC_MASK);
                    event_type = MALIBU10G_EXTENDED2_EVENT;
                    break;
                case 6:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    ex2_ev = (1 << VTSS_PHY_HOST_1G_XGMII_MASK_LINK_DOWN_MASK) | (1 << VTSS_PHY_HOST_1G_XGMII_MASK_OUT_OF_SYNC_MASK);
                    event_type = MALIBU10G_EXTENDED2_EVENT;
                    break;
                case 7:
                case 8:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event = MEPA_MACSEC_SEQ_ROLLOVER_EVENT;
                    event_type = MALIBU10G_MACSEC_EVENT;
                    break;
                case 9:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_LINE_MAC_LOCAL_FAULT_EV | VTSS_PHY_10G_LINE_MAC_REMOTE_FAULT_EV;
                    break;
                case 10:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_HOST_MAC_LOCAL_FAULT_EV | VTSS_PHY_10G_HOST_MAC_REMOTE_FAULT_EV;
                    break;
                case 11:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED2_EVENT;
                    ex2_ev = (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_RX_UNDERFLOW_DROP_STICKY_MASK) | (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_RX_OVERFLOW_DROP_STICKY_MASK) |
                             (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_TX_DATA_QUEUE_UNDERFLOW_DROP_STICKY_MASK) |
                             (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_TX_DATA_QUEUE_OVERFLOW_DROP_STICKY_MASK)  |
                             (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_TX_UNCORRECTED_FRM_DROP_STICKY_MASK)      |
                             (1 << VTSS_MAC_FC_BUFFER_STATUS_MASK_RX_UNCORRECTED_FRM_DROP_STICKY_MASK);
                    break;
                case 12:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_RX_FIFO_UNDERFLOW_EV | VTSS_PHY_10G_RX_FIFO_OVERFLOW_EV;
                    break;
                case 13:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_TX_FIFO_UNDERFLOW_EV | VTSS_PHY_10G_TX_FIFO_OVERFLOW_EV;
                    break;
                case 14:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_TX_FIFO2_UNDERFLOW_EV | VTSS_PHY_10G_TX_FIFO2_OVERFLOW_EV;
                    break;
                case 15:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED2_EVENT;
                    ex2_ev = (1 << VTSS_PHY_LINE_10G_RX_LOS_EV) | (1 << VTSS_PHY_LINE_10G_RX_LOL_EV) | (1 << VTSS_PHY_LINE_10G_TX_LOL_EV);
                    break;
                case 16:
                    gpio_mode.aggr_intrpt = 1 << ((channel_id * 2) + 1);
                    event_type = MALIBU10G_EXTENDED_EVENT;
                    ext_ev = VTSS_PHY_10G_RX_LOS_EV | VTSS_PHY_10G_RX_LOL_EV | VTSS_PHY_10G_TX_LOL_EV;
                    break;
                case 17:
                    gpio_mode.aggr_intrpt =  1 << (channel_id + 8);
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_1588_INT;
                    event_type = MALIBU10G_PTP_EVENT;
                    ts_ev = VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED | VTSS_PHY_TS_EGR_ENGINE_ERR | VTSS_PHY_TS_EGR_FIFO_OVERFLOW | VTSS_PHY_TS_INGR_ENGINE_ERR |
                            VTSS_PHY_TS_INGR_RW_FCS_ERR | VTSS_PHY_TS_EGR_RW_FCS_ERR;
                    break;
                case 18:
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_PLL_INT_0;
                    break;
                case 19:
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_PLL_INT_1;
                    break;
                case 20:
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_CRSS_INT;
                    break;
                default:
                    T_E("\n Invalid Input \n");
                    return;
                }
                cli_printf("\n\t Event Enable or Disable [1 or 0] : ");
                scanf("%d", &enable);
                event_ena_dis = (enable == 1) ? 1 : 0;
            }


        } else {
            T_E("\n Invalid Input \n");
            return;
        }

        if (vtss_phy_10g_gpio_mode_set(NULL, req->port_no, gpio_no, &gpio_mode) != VTSS_RC_OK) {
            T_E("\n Error in Configuring GPIO on port : %d\n", req->port_no);
            return;
        }


        if (event_type == MALIBU10G_EVENT) {
            if (vtss_phy_10g_event_enable_set(NULL, req->port_no, ev, event_ena_dis) != VTSS_RC_OK) {
                T_E("\n vtss_phy_10g_event_enable_set, port %d, gpio %d\n", req->port_no, gpio_no);
                return;
            }
        } else if (event_type == MALIBU10G_EXTENDED_EVENT) {
            if (vtss_phy_10g_extended_event_enable_set(NULL, req->port_no, ext_ev, event_ena_dis) != VTSS_RC_OK) {
                T_E("\n vtss_phy_10g_extended_event_enable_set, port %d, gpio %d\n", req->port_no, gpio_no);
                return;
            }
        } else if (event_type == MALIBU10G_EXTENDED2_EVENT) {
            if (vtss_phy_10g_extended2_event_enable_set(NULL, req->port_no, ex2_ev, event_ena_dis) != VTSS_RC_OK) {
                T_E("\n vtss_phy_10g_extended2_event_enable_set, port %d, gpio %d\n", req->port_no, gpio_no);
                return;
            }
        } else if (event_type == MALIBU10G_MACSEC_EVENT) {
            if ((rc = mepa_macsec_event_enable_set(meba_gpio_lp_instance->phy_devices[req->port_no], req->port_no, event, event_ena_dis)) != MEPA_RC_OK) {
                T_E("\n Error in Configuring MACsec Event on Port : %d \n", req->port_no);
                return;
            }
        } else if (event_type == MALIBU10G_PTP_EVENT) {
            if (vtss_phy_ts_event_enable_set(NULL, req->port_no, event_ena_dis, ts_ev) != VTSS_RC_OK) {
                T_E("\n vtss_phy_ts_event_enable_set, port %d, gpio %d\n", req->port_no, gpio_no);
                return;
            }
        }
        cli_printf("\n GPIO Pin %d configured in %s Mode......\n", gpio_no, gpio_txt[gpio_conf - 1]);
    } else if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
        mepa_bool_t pp_enable;
        uint8_t check = 0;

        memset(&gpio_conf, 0, sizeof(mepa_gpio_conf_t));
        cli_printf("\n");
        cli_printf("\t 1 . Output Mode\n");
        cli_printf("\t 2 . Input Mode\n");
        cli_printf("\t 3 . Alternate Mode\n");

        cli_printf("\t Enter the Mode of GPIO Pin : ");
        check = scanf("%d", &gpio_mode);
        if (check != 1) {
            T_E("Invalid input! Only integers are allowed.\n");
            return;
        }

        if (gpio_mode == 1) {
            gpio_conf.mode = MEPA_GPIO_MODE_OUT;
        } else if (gpio_mode == 2) {
            gpio_conf.mode = MEPA_GPIO_MODE_IN;
        } else if (gpio_mode == 3) {
            gpio_conf.mode = MEPA_GPIO_MODE_ALT;
        } else {
            T_E("\n Invalid Mode Selected \n");
            return;
        }

        cli_printf("\n\t Enter GPIO Number to be configured as %s mode: ", gpio_txt[gpio_mode - 1]);
        check = scanf("%hhu", &gpio_num);
        if (check != 1) {
            T_E("Invalid input! Only integers are allowed.\n");
            return;
        }
        if (gpio_num > LAN80XX_MAX_GPIO_NO) {
            T_E("Maximum supported GPIO are [0 - 40]\n");
            return;
        }
        gpio_conf.gpio_no = gpio_num;
        if (gpio_conf.mode == MEPA_GPIO_MODE_ALT) {
            if ((gpio_num == 36) || (gpio_num == 37) || (gpio_num == 38) || (gpio_num == 39)) {
                gpio_conf.led_num = MEPA_LED1;
            } else {
                gpio_conf.led_num = MEPA_LED0;
            }
        }
        cli_printf("\n");
        cli_printf("\t Enter the GPIO in 1 - Push pull mode or 0 - Open drain mode : ");
        scanf("%hhd", &pp_enable);
        gpio_conf.pp_enable =  (pp_enable == 1) ? 1 : 0;
        if (gpio_conf.mode == MEPA_GPIO_MODE_IN) {
            cli_printf("\n");
            cli_printf("\t Enter the GPIO interrupt [0 -  GPIO_INTR_NONE or 1 - GPIO_INTR0 or 2 - GPIO_INTR1 : ");
            check = scanf("%d", &gpio_intr);
            if (check != 1) {
                T_E("Invalid input! Only integers are allowed.\n");
                return;
            }
            if (gpio_intr > 2) {
                T_E("Invalid GPIO_INTR configured\n");
                return;
            }
            switch (gpio_conf.mode) {
            case MEPA_GPIO_MODE_OUT:
            case MEPA_GPIO_MODE_ALT:
                if ((gpio_intr == MEPA_GPIO_INTR_1) || (gpio_intr == MEPA_GPIO_INTR_0)) {
                    T_E("GPIO INTR 0-1 should be configured while in Input mode\n");
                    return;
                }
                break;
            default:
                break;
            }
            gpio_conf.gpio_intrpt = gpio_intr;
        }
        if ((rc = mepa_gpio_mode_set(meba_gpio_lp_instance->phy_devices[req->port_no], &gpio_conf)) != MEPA_RC_OK) {
            T_E(" Error in configuring GPIO : %d\n", req->port_no);
            return;
        }
    }
    cli_printf("GPIO Configured succesfully\n");
    return;
}

static void event_poll(meba_inst_t inst)
{
    mepa_port_no_t port_no;
    phy25g_events_t evt_get;
    phy25g_ext_events_t ext_evt_get;
    mepa_rc rc;

    for (port_no = 0; port_no < MAX_PORTS; port_no++) {
        if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, port_no)) != MEPA_RC_OK) {
            continue;
        }
        if (event_poll_en[port_no] != TRUE) {
            continue;
        }
        evt_get = 0;
        if ((rc = lan80xx_event_poll(meba_gpio_lp_instance->phy_devices[port_no], port_no, &evt_get)) != MEPA_RC_OK) {
            T_E("\n Error in Polling the Events on port : %d \n", port_no);
            return;
        }
        if (evt_get != 0) {
            for (int pos = 0; pos <= LAN80XX_MAX_EVENTS; pos++) {
                if ((evt_get >> pos) & 1) {
                    printf("Event: %s occured on port %d\n", event_list[pos], port_no);
                }
            }
        }
        ext_evt_get = 0;
        if ((rc = lan80xx_ext_event_poll(meba_gpio_lp_instance->phy_devices[port_no], port_no, &ext_evt_get)) != MEPA_RC_OK) {
            T_E("\n Error in Polling the Extended Events on port : %d \n", port_no);
            return;
        }
        if (ext_evt_get != 0) {
            for (int pos = 0; pos <= LAN80XX_MAX_EVENTS; pos++) {
                if ((ext_evt_get >> pos) & 1) {
                    printf("Extented event: %s occured on port %d\n", ext_event_list[pos], port_no);
                }
            }
        }
    }
}

static void cli_cmd_event_poll(cli_req_t *req)
{

    event_poll_en[req->port_no] = req->enable;
    if (req->enable) {
        cli_printf("Event polling got enabled on port %d\n", req->port_no);
    } else {
        cli_printf("Event polling got disabled on port %d\n", req->port_no);
    }
    return;
}

static void cli_cmd_gpio_out_set(cli_req_t *req)
{
    mepa_rc rc;
    gpio_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_out_set(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, req->enable)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Write : %d\n", req->port_no);
        return;
    }
    return;
}

static void cli_cmd_gpio_in_get(cli_req_t *req)
{
    mepa_rc rc;
    gpio_configuration *mreq = req->module_req;;
    mepa_bool_t value;
    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_in_get(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, &value)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Read : %d\n", req->port_no);
        return;
    }
    cli_printf("\n Value of GPIO %d is : %d\n", mreq->gpio_number, value);
    return;
}

static void cli_cmd_gpio_toggle(cli_req_t *req)
{
    mepa_rc rc;
    gpio_configuration *mreq = req->module_req;;
    mepa_bool_t value;
    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_in_get(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, &value)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Read : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_out_set(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, !value)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Write : %d\n", req->port_no);
        return;
    }
    return;
}

static void cli_cmd_event_conf_set(cli_req_t *req)
{
    mepa_rc rc;
    event_configuration_t *mreq = req->module_req;
    phy25g_event_conf_t  evt_conf;
    phy25g_ext_event_conf_t  ext_evt_conf;
    uint8_t evt;

    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = lan80xx_event_conf_get(meba_gpio_lp_instance->phy_devices[req->port_no], req->port_no, &evt_conf.evt_set)) != MEPA_RC_OK) {
        cli_printf(" Error in Getting Event configuration : %d\n", req->port_no);
        return;
    }
    if ((rc = lan80xx_ext_event_conf_get(meba_gpio_lp_instance->phy_devices[req->port_no], req->port_no, &ext_evt_conf.ext_evt_set)) != MEPA_RC_OK) {
        cli_printf(" Error in Getting Extended event configuration : %d\n", req->port_no);
        return;
    }
    for (evt = 0; evt < mreq->event_cnt; evt++) {
        if (mreq->event_list[evt] <= LAN80XX_MAX_EVENTS) {
            evt_conf.evt_set = req->enable ? (evt_conf.evt_set | ((uint64_t)1 << mreq->event_list[evt])) : (evt_conf.evt_set & ~((uint64_t)1 << mreq->event_list[evt]));
        } else if ((mreq->event_list[evt] <= LAN80XX_TOTAL_MAX_EVENTS) && (mreq->event_list[evt] >= (LAN80XX_TOTAL_MAX_EVENTS - LAN80XX_MAX_EVENTS))) {
            ext_evt_conf.ext_evt_set = req->enable ? (ext_evt_conf.ext_evt_set | ((uint64_t)1 << (mreq->event_list[evt] - (LAN80XX_TOTAL_MAX_EVENTS - LAN80XX_MAX_EVENTS)))) : (ext_evt_conf.ext_evt_set & ~((uint64_t)1 << (mreq->event_list[evt] - (LAN80XX_TOTAL_MAX_EVENTS - LAN80XX_MAX_EVENTS))));
        } else {
            cli_printf(" Invalid Event configuration %d\n", req->port_no);
        }
    }
    ext_evt_conf.enable = evt_conf.enable = req->enable;
    ext_evt_conf.intr_sel = evt_conf.intr_sel = mreq->intr_a;
    if ((rc = lan80xx_event_conf_set(meba_gpio_lp_instance->phy_devices[req->port_no], req->port_no, evt_conf)) != MEPA_RC_OK) {
        cli_printf(" Error in Event configuration : %d\n", req->port_no);
        return;
    }
    if ((rc = lan80xx_ext_event_conf_set(meba_gpio_lp_instance->phy_devices[req->port_no], req->port_no, ext_evt_conf)) != MEPA_RC_OK) {
        cli_printf(" Error in Extended event configuration : %d\n", req->port_no);
        return;
    }
    cli_printf("Event configured successfully\n");
    return;
}

static int cli_parm_value_event(cli_req_t *req)
{
    event_configuration_t *mreq = req->module_req;
    int ret = cli_parse_values(req->cmd, mreq->event_list, &mreq->event_cnt, 0, 127, 127);
    if (mreq->event_cnt > LAN80XX_TOTAL_MAX_EVENTS) {
        T_E("Only event (0-63) and extended event (64-127) can be configured\n");
        return ERROR;
    }
    return ret;
}

static int cli_parm_keyword(cli_req_t *req)
{
    const char     *found;
    event_configuration_t *mreq = req->module_req;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        return 1;
    }
    if (!strncasecmp(found, "intr_a", strlen(req->cmd))) {
        mreq->intr_a = 0;
    }
    if (!strncasecmp(found, "intr_b", strlen(req->cmd))) {
        mreq->intr_a = 1;
    }
    return 0;
}



static int cli_param_parse_gpio_no(cli_req_t *req)
{
    gpio_configuration *mreq = req->module_req;
    return cli_parm_u8(req, &mreq->gpio_number, 0, 0x28);
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "gpio_conf <port_no>",
        "GPIO Configuration",
        cli_cmd_gpio_conf,
    },

    {
        "gpio_out_set <port_no> <gpio_num> [enable|disable]",
        "GPIO Output Pin Set",
        cli_cmd_gpio_out_set,
    },

    {
        "gpio_in_get <port_no> <gpio_num>",
        "GPIO Value Get",
        cli_cmd_gpio_in_get,
    },

    {
        "gpio_toggle <port_no> <gpio_num>",
        "GPIO toggle",
        cli_cmd_gpio_toggle,
    },

    {
        "event_set <port_no> <event_list> [enable|disable] [intr_a|intr_b]",
        "Event set configuration",
        cli_cmd_event_conf_set,
    },

    {
        "event_poll <port_no> [enable|disable]",
        "Poll all events",
        cli_cmd_event_poll,
    },
};


static cli_parm_t cli_parm_table[] = {
    {
        "<gpio_num>",
        "GPIO Number",
        CLI_PARM_FLAG_NONE,
        cli_param_parse_gpio_no,
    },

    {
        "<event_list>",
        "event_list   : Set the events and extended events list\n"
        "Below mentioned are the list of events:\n"

        "127 - L3_FIFO_ERROR_INTR\n"
        "126 - L2_FIFO_ERROR_INTR\n"
        "125 - L1_FIFO_ERROR_INTR\n"
        "124 - L0_FIFO_ERROR_INTR\n"
        "123 - H3_FIFO_ERROR_INTR\n"
        "122 - H2_FIFO_ERROR_INTR\n"
        "121 - H1_FIFO_ERROR_INTR\n"
        "120 - H0_FIFO_ERROR_INTR\n"
        "119 - L3_COND_ALT_UNF_DET_INTR\n"
        "118 - L2_COND_ALT_UNF_DET_INTR\n"
        "117 - L1_COND_ALT_UNF_DET_INTR\n"
        "116 - L0_COND_ALT_UNF_DET_INTR\n"
        "115 - H3_COND_ALT_UNF_DET_INTR\n"
        "114 - H2_COND_ALT_UNF_DET_INTR\n"
        "113 - H1_COND_ALT_UNF_DET_INTR\n"
        "112 - H0_COND_ALT_UNF_DET_INTR\n"
        "111 - L3_COND_ALT_DET_INTR\n"
        "110 - L2_COND_ALT_DET_INTR\n"
        "109 - L1_COND_ALT_DET_INTR\n"
        "108 - L0_COND_ALT_DET_INTR\n"
        "107 - H3_COND_ALT_DET_INTR\n"
        "106 - H2_COND_ALT_DET_INTR\n"
        "105 - H1_COND_ALT_DET_INTR\n"
        "104 - H0_COND_ALT_DET_INTR\n"
        "103 - L3_SWITCH_INTR\n"
        "102 - L2_SWITCH_INTR\n"
        "101 - L1_SWITCH_INTR\n"
        "100 - L0_SWITCH_INTR\n"
        "99 - H3_SWITCH_INTR\n"
        "98 - H2_SWITCH_INTR\n"
        "97 - H1_SWITCH_INTR\n"
        "96 - H0_SWITCH_INTR\n"
        "95 - WPS0_FAILOVER_INTR\n"
        "94 - WPS1_FAILOVER_INTR\n"
        "93 - WPS0_CONN_FAULT_INTR\n"
        "92 - WPS1_CONN_FAULT_INTR\n"
        "91 - WPS0_FC_ACK_TIMER_INTR\n"
        "90 - WPS1_FC_ACK_TIMER_INTR\n"
        "89 - LINE_PMA_RXEI_FILTERED_INTR\n"
        "88 - LINE_PMA_RESET_DONE_INTR\n"
        "87 - HOST_PMA_RXEI_FILTERED_INTR\n"
        "86 - HOST_PMA_RESET_DONE_INTR\n"
        "85 - LINE_MAC_MM_PRMPT_UNEXP_TX_PFRM_INTR\n"
        "84 - LINE_MAC_MM_PRMPT_UNEXP_RX_PFRM_INTR\n"
        "83 - LINE_MAC_MM_PRMPT_ACTIVE_INTR\n"
        "82 - HOST_MAC_MM_PRMPT_UNEXP_TX_PFRM_INTR\n"
        "81 - HOST_MAC_MM_PRMPT_UNEXP_RX_PFRM_INTR\n"
        "80 - HOST_MAC_MM_PRMPT_ACTIVE_INTR\n"
        "79 - LINE_PMAC_RX_PREAMBLE_SHRINK\n"
        "78 - LINE_PMAC_RX_PREAMBLE_MISMATCH\n"
        "77 - LINE_PMAC_RX_PREAMBLE_ERR\n"
        "76 - LINE_PMAC_RX_NON_STD_PREAMBLE_INTR\n"
        "75 - LINE_PMAC_RX_MPLS_MC_INTR\n"
        "74 - LINE_PMAC_RX_MPLS_UC_INTR\n"
        "73 - LINE_PMAC_TX_UNDERFLOW_INTR\n"
        "72 - LINE_PMAC_TX_ABORT_INTR\n"
        "71 - HOST_PMAC_RX_PREAMBLE_SHRINK\n"
        "70 - HOST_PMAC_RX_PREAMBLE_MISMATCH\n"
        "69 - HOST_PMAC_RX_PREAMBLE_ERR\n"
        "68 - HOST_PMAC_RX_NON_STD_PREAMBLE_INTR\n"
        "67 - HOST_PMAC_RX_MPLS_MC_INTR\n"
        "66 - HOST_PMAC_RX_MPLS_UC_INTR\n"
        "65 - HOST_PMAC_TX_UNDERFLOW_INTR\n"
        "64 - HOST_PMAC_TX_ABORT_INTR\n"
        "63 - GPIO_INTR_1\n"
        "62 - GPIO_INTR_0\n"
        "61 - LINE_MAC_LOCAL_ERR_STATE_INTR\n"
        "60 - LINE_MAC_REMOTE_ERR_STATE_INTR\n"
        "59 - LINE_MAC_IDLE_STATE_INTR\n"
        "58 - LINE_MAC_DIS_STATE_INTR\n"
        "57 - HOST_MAC_LOCAL_ERR_STATE_INTR\n"
        "56 - HOST_MAC_REMOTE_ERR_STATE_INTR\n"
        "55 - HOST_MAC_IDLE_STATE_INTR\n"
        "54 - HOST_MAC_DIS_STATE_INTR\n"
        "53 - LINE_MAC_RX_IPG_SHRINK_INTR\n"
        "52 - LINE_MAC_RX_PREAMBLE_SHRINK_INTR\n"
        "51 - LINE_MAC_RX_PREAMBLE_MISMATCH_INTR\n"
        "50 - LINE_MAC_RX_PREAM_ERR_INTR\n"
        "49 - LINE_MAC_RX_NON_STD_PREAMBLE_INTR\n"
        "48 - LINE_MAC_RX_MPLS_MC_INTR\n"
        "47 - LINE_MAC_RX_MPLS_UC_INTR\n"
        "46 - LINE_MAC_RX_TAG_INTR\n"
        "45 - LINE_MAC_TX_UNDERFLOW_INTR\n"
        "44 - LINE_MAC_TX_ABORT_INTR\n"
        "43 - HOST_MAC_RX_IPG_SHRINK_INTR\n"
        "42 - HOST_MAC_RX_PREAMBLE_SHRINK_INTR\n"
        "41 - HOST_MAC_RX_PREAMBLE_MISMATCH_INTR\n"
        "40 - HOST_MAC_RX_PREAM_ERR_INTR\n"
        "39 - HOST_MAC_RX_NON_STD_PREAMBLE_INTR\n"
        "38 - HOST_MAC_RX_MPLS_MC_INTR\n"
        "37 - HOST_MAC_RX_MPLS_UC_INTR\n"
        "36 - HOST_MAC_RX_TAG_INTR\n"
        "35 - HOST_MAC_TX_UNDERFLOW_INTR\n"
        "34 - HOST_MAC_TX_ABORT_INTR\n"
        "33 - EGR_FC_BUFFER_FAULT_READ\n"
        "32 - EGR_FC_BUFFER_DED_INTR\n"
        "31 - EGR_FC_BUFFER_SEC_INTR\n"
        "30 - INGR_FC_BUFFER_FAULT_READ_INTR\n"
        "29 - INGR_FC_BUFFER_DED_INTR\n"
        "28 - INGR_FC_BUFFER_SEC_INTR\n"
        "27 - EGR_FC_BUFFER_INIT_DONE_INTR\n"
        "26 - INGR_FC_BUFFER_INIT_DONE_INTR\n"
        "25 - EGR_ECC_INTERRUPT_STATUS_INTR\n"
        "24 - INGR_ECC_INTERRUPT_STATUS_INTR\n"
        "23 - RX_UNDERFLOW_DROP_INTR\n"
        "22 - RX_OVERFLOW_DROP_INTR\n"
        "21 - TX_DATA_QUEUE_UNDERFLOW_DROP_INTR\n"
        "20 - TX_DATA_QUEUE_OVERFLOW_DROP_INTR\n"
        "19 - TX_CTRL_QUEUE_UNDERFLOW_DROP_INTR\n"
        "18 - TX_CTRL_QUEUE_OVERFLOW_DROP_INTR\n"
        "17 - RX_UNCORRECTED_FRM_DROP_INTR\n"
        "16 - TX_UNCORRECTED_FRM_DROP_INTR\n"
        "15 - XON_PAUSE_GEN_INTR\n"
        "14 - XOFF_PAUSE_GEN_INTR\n"
        "13 - L2H_RA_FIFO_OVERFLOW_INTR\n"
        "12 - L2H_RA_FIFO_UNDERFLOW_INTR\n"
        "11 - H2L_RA_FIFO_OVERFLOW_INTR\n"
        "10 - H2L_RA_FIFO_UNDERFLOW_INTR\n"
        "9 - HOST_PCS25G_HI_BER_INTR\n"
        "8 - HOST_PCS25G_BLOCK_LOCK_INTR\n"
        "7 - HOST_PCS25G_ALIGN_DONE_INTR\n"
        "6 - LINE_PCS25G_HI_BER_INTR\n"
        "5 - LINE_PCS25G_BLOCK_LOCK_INTR\n"
        "4 - LINE_PCS25G_ALIGN_DONE_INTR\n"
        "3 - HOST_PCS1G_OUT_OF_SYNC_INTR\n"
        "2 - HOST_PCS1G_LINK_DOWN_INTR\n"
        "1 - LINE_PCS1G_OUT_OF_SYNC_INTR\n"
        "0 - LINE_PCS1G_LINK_DOWN_INTR\n",
        CLI_PARM_FLAG_SET,
        cli_parm_value_event,
    },

    {
        "intr_a|intr_b",
        "intr_a	: Enable aggregate interrupt A\n"
        "intr_b	: Enable aggregate interrupt B\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    }

};

static void phy_cli_init(void)
{
    int i;

    /* Register CLI Commands */
    for (i = 0; i < sizeof(cli_cmd_table) / sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

    /* Register CLI Params */
    for (i = 0; i < sizeof(cli_parm_table) / sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mepa_demo_appl_gpio_lp_demo(mscc_appl_init_t *init)
{
    meba_gpio_lp_instance = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;
    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;
    case MSCC_INIT_CMD_POLL:
        event_poll(init->board_inst);
        break;
    default:
        break;
    }

}
