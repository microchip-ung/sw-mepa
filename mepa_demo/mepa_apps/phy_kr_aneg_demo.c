// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "mesa-rpc.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "phy_demo_apps.h"
#include "lan80xx.h"
#include <unistd.h>
#include "lan80xx_mcu.h"

meba_inst_t meba_phy_kr_inst;

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_kr_aneg"
};

enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};

static mscc_appl_trace_group_t trace_groups[TRACE_GROUP_CNT] = {
    // TRACE_GROUP_DEFAULT
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

typedef struct {
    mesa_bool_t adv1g;
    mesa_bool_t adv10g;
    mesa_bool_t adv25g_kr;
    mesa_bool_t adv25g_krs;
    mesa_bool_t rfec_10g;
    mesa_bool_t rfec_25g;
    mesa_bool_t rsfec_25g;
    mesa_bool_t np;
    mesa_bool_t np_rfec;
    mesa_bool_t np_rsfec;
    mesa_bool_t fw_res;
    mesa_bool_t train;
    uint32_t    value;
    mesa_bool_t dis;
} phy_kr_cli_req_t;

typedef struct phy_sd_cli_req {
    // 0 - 1G, 1 - 10G, 2 - 25G
    uint8_t    speed_idx;

} phy_sd_cli_req_t;

////////////////////////////////////////////////////////////
/////////////////   CLI Command functions   ////////////////
////////////////////////////////////////////////////////////

static mepa_rc Islinkup(int port_no, mepa_adv_side_t port_type, u8 *speed)
{
    mepa_rc rc = MESA_RC_ERROR;
    uint32_t value = 0, mmd = 0;

    if (port_type == MEPA_ADV_SIDE_LINE) {
        mmd = 0x07;
    } else if (port_type == MEPA_ADV_SIDE_HOST) {
        mmd = 0x0F;
    } else {
        return rc;
    }

    if ((rc = lan80xx_phy_csr_read(meba_phy_kr_inst->phy_devices[port_no], port_no, mmd, 0x01, &value)) != MEPA_RC_OK) {
        T_E("\n Error Reading a Register on port : %d \n", (port_no));
        return rc;
    }
    cli_printf ("AN_STS0: 0x%02X\n", value);
    if (value & 0x20) {
        cli_printf("ANEG completed\n");
    } else {
        cli_printf ("Link is not up\n");
        return MESA_RC_ERROR;
    }

    if ((rc = lan80xx_phy_csr_read(meba_phy_kr_inst->phy_devices[port_no], port_no, mmd, 0x8032, &value)) != MEPA_RC_OK) {
        T_E("\n Error Reading a Register on port : %d \n", (port_no));
        return MESA_RC_ERROR;
    }
    cli_printf ("AN_STS1: 0x%02X\n", value);
    *speed = value & 0x0f;

    rc = MESA_RC_OK;
    return rc;
}

void dump_sd_cfg(__SERDES_CONFIG_T cfg, eSERDES_CFG_T cfgType)
{
    switch (cfgType) {
    case eTX_EQ_CFG:
        printf("TX EQ Config:\n");
        printf("  Tap_dly: 0x%02X\n", cfg.sTx_eq_cfg.Tap_dly);
        printf("  Tap_main: 0x%02X\n", cfg.sTx_eq_cfg.Tap_main);
        printf("  Tap_adv: 0x%02X\n", cfg.sTx_eq_cfg.Tap_adv);
        printf("  En_main: 0x%02X\n", cfg.sTx_eq_cfg.En_main);
        printf("  En_adv: 0x%02X\n", cfg.sTx_eq_cfg.En_adv);
        printf("  En_dly: 0x%02X\n", cfg.sTx_eq_cfg.En_dly);
        break;
    case eCDR_CFG:
        printf("CDR Config:\n");
        printf("  Cdr_m: 0x%02X\n", cfg.sCdr_cfg.Cdr_m);
        printf("  Alos_thr: 0x%02X\n", cfg.sCdr_cfg.Alos_thr);
        break;

    case eSPEED_CHANGE_CFG:
        printf("Speed Change Config:\n");
        printf("  L0_cfg_tx_reserve_15_8: 0x%02X\n", cfg.sSpeed_change_cfg.L0_cfg_tx_reserve_15_8);
        printf("  L0_cfg_tx_reserve_7_0: 0x%02X\n", cfg.sSpeed_change_cfg.L0_cfg_tx_reserve_7_0);
        printf("  Ln_cfg_tx_reserve_15_8: 0x%02X\n", cfg.sSpeed_change_cfg.Ln_cfg_tx_reserve_15_8);
        printf("  Ln_cfg_tx_reserve_7_0: 0x%02X\n", cfg.sSpeed_change_cfg.Ln_cfg_tx_reserve_7_0);
        break;

    case eRX_EQ_CFG:
        printf("RX EQ Config:\n");
        printf("  Ln_cfg_vga_ctrl_byp: 0x%02X\n", cfg.sRx_eq_cfg.Ln_cfg_vga_ctrl_byp);
        printf("  Ln_cfg_vga_byp: 0x%02X\n", cfg.sRx_eq_cfg.Ln_cfg_vga_byp);
        printf("  Ln_cfg_eqr_force: 0x%02X\n", cfg.sRx_eq_cfg.Ln_cfg_eqr_force);
        printf("  Ln_cfg_eqc_force: 0x%02X\n", cfg.sRx_eq_cfg.Ln_cfg_eqc_force);
        break;

    case eDFE_CFG:
        printf("DFE Config:\n");
        printf("  Ln_cfg_dfeck_en: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfeck_en);
        printf("  Ln_cfg_dfe_pd: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfe_pd);
        printf("  Ln_cfg_dfemx_pd: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfemx_pd);
        printf("  Ln_cfg_dfe_dmux_pd: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfe_dmux_pd);
        printf("  Ln_cfg_pi_dfe_en: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_pi_dfe_en);
        printf("  Ln_cfg_dfedig_m: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfedig_m);
        printf("  Ln_cfg_dfetap_en: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_dfetap_en);
        printf("  Ln_cfg_en_dfedig: 0x%02X\n", cfg.sDfe_cfg.Ln_cfg_en_dfedig);
        break;

    case ePOL_SQ_CFG:
        printf("Polarity and Signal Quality Config:\n");
        printf("  Tx_pol_inv: 0x%02X\n", cfg.sPol_sq_cfg.Tx_pol_inv);
        printf("  Rx_pol_inv: 0x%02X\n", cfg.sPol_sq_cfg.Rx_pol_inv);
        printf("  Ln_cfg_dis_sq: 0x%02X\n", cfg.sPol_sq_cfg.Ln_cfg_dis_sq);
        printf("  Ln_cfg_pd_sq: 0x%02X\n", cfg.sPol_sq_cfg.Ln_cfg_pd_sq);
        break;

    case eTX_SWING_CFG:
        printf("TX Swing Config:\n");
        printf("  Itx_ipdriver_base: 0x%02X\n", cfg.sTx_swing_cfg.Itx_ipdriver_base);
        break;

    case eMISC_CFG:
        printf("Misc Config:\n");
        printf("  Cfg_common_reserve: 0x%02X\n", cfg.sMisc_cfg.Cfg_common_reserve);
        printf("  Cfg_jc_byp: 0x%02X\n", cfg.sMisc_cfg.Cfg_jc_byp);
        printf("  Cfg_pll_lol_set: 0x%02X\n", cfg.sMisc_cfg.Cfg_pll_lol_set);
        printf("  Cfg_en_dummy: 0x%02X\n", cfg.sMisc_cfg.Cfg_en_dummy);
        printf("  Cfg_pll_reserve: 0x%02X\n", cfg.sMisc_cfg.Cfg_pll_reserve);
        printf("  R_dwidthctrl_from_hwt: 0x%02X\n", cfg.sMisc_cfg.R_dwidthctrl_from_hwt);
        printf("  R_reg_manual: 0x%02X\n", cfg.sMisc_cfg.R_reg_manual);
        printf("  Vco_div_mode: 0x%02X\n", cfg.sMisc_cfg.Vco_div_mode);
        printf("  Pre_divsel: 0x%02X\n", cfg.sMisc_cfg.Pre_divsel);
        printf("  Cfg_seldiv: 0x%02X\n", cfg.sMisc_cfg.Cfg_seldiv);
        printf("  Data_width_sel: 0x%02X\n", cfg.sMisc_cfg.Data_width_sel);
        printf("  Txfifo_ck_div: 0x%02X\n", cfg.sMisc_cfg.Txfifo_ck_div);
        printf("  Rxfifo_ck_div: 0x%02X\n", cfg.sMisc_cfg.Rxfifo_ck_div);
        printf("  Pma_txck_sel: 0x%02X\n", cfg.sMisc_cfg.Pma_txck_sel);
        printf("  Tx_prediv: 0x%02X\n", cfg.sMisc_cfg.Tx_prediv);
        printf("  Rxdiv_sel: 0x%02X\n", cfg.sMisc_cfg.Rxdiv_sel);
        printf("  Txrate_sel: 0x%02X\n", cfg.sMisc_cfg.Txrate_sel);
        printf("  Rxrate_sel: 0x%02X\n", cfg.sMisc_cfg.Rxrate_sel);
        printf("  Ln_cfg_cdrck_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_cdrck_en);
        printf("  Ln_cfg_dmux_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_dmux_pd);
        printf("  Ln_cfg_dmux_clk_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_dmux_clk_pd);
        printf("  Ln_cfg_erramp_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_erramp_pd);
        printf("  Ln_cfg_pi_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pi_en);
        printf("  Ln_cfg_pd_ctle: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pd_ctle);
        printf("  Ln_cfg_summer_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_summer_en);
        printf("  Ln_cfg_pmad_ck_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pmad_ck_pd);
        printf("  Ln_cfg_pd_clk: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pd_clk);
        printf("  Ln_cfg_pd_cml: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pd_cml);
        printf("  Ln_cfg_pd_driver: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pd_driver);
        printf("  Ln_cfg_rx_reg_pu: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_rx_reg_pu);
        printf("  Ln_cfg_pd_rms_det: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pd_rms_det);
        printf("  Ln_cfg_dcdr_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_dcdr_pd);
        printf("  Ln_cfg_ecdr_pd: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_ecdr_pd);
        printf("  L0_cfg_bw: 0x%02X\n", cfg.sMisc_cfg.L0_cfg_bw);
        printf("  L0_cfg_txcal_en: 0x%02X\n", cfg.sMisc_cfg.L0_cfg_txcal_en);
        printf("  Ln_cfg_bw: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_bw);
        printf("  Ln_cfg_txcal_man_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_txcal_man_en);
        printf("  Ln_cfg_phase_man: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_phase_man);
        printf("  Ln_cfg_quad_man: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_quad_man);
        printf("  Ln_cfg_txcal_shift_code: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_txcal_shift_code);
        printf("  Ln_cfg_txcal_valid_sel: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_txcal_valid_sel);
        printf("  Ln_cfg_cdr_kf: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_cdr_kf);
        printf("  Ln_cfg_pi_bw: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pi_bw);
        printf("  Ln_cfg_pi_steps: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_pi_steps);
        printf("  Ln_cfg_dis_2ndorder: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_dis_2ndorder);
        printf("  Ln_cfg_rx_reserve_7_0: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_rx_reserve_7_0);
        printf("  Ln_cfg_rx_reserve_15_8: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_rx_reserve_15_8);
        printf("  Ln_cfg_rx_term: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_rx_term);
        printf("  Ln_cfg_rx_sp_ctle: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_rx_sp_ctle);
        printf("  Ln_cfg_isel_ctle: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_isel_ctle);
        printf("  Ln_cfg_eqr_byp: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_eqr_byp);
        printf("  Ln_cfg_agc_adpt_byp: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_agc_adpt_byp);
        printf("  Ln_cfg_sum_setcm_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_sum_setcm_en);
        printf("  Ln_cfg_init_pos_iscan: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_init_pos_iscan);
        printf("  Ln_cfg_init_pos_iPI: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_init_pos_iPI);
        printf("  Cfg_i_vco: 0x%02X\n", cfg.sMisc_cfg.Cfg_i_vco);
        printf("  Icp_base_sel: 0x%02X\n", cfg.sMisc_cfg.Icp_base_sel);
        printf("  Icp_sel: 0x%02X\n", cfg.sMisc_cfg.Icp_sel);
        printf("  Cfg_rsel: 0x%02X\n", cfg.sMisc_cfg.Cfg_rsel);
        printf("  Ln_cfg_iscan_en: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_iscan_en);
        printf("  Ln_cfg_en_fast_iscan: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_en_fast_iscan);
        printf("  Ln_cfg_filter2nd_yz_6_0: 0x%02X\n", cfg.sMisc_cfg.Ln_cfg_filter2nd_yz_6_0);
        printf("  Ln_r_alos_en: 0x%02X\n", cfg.sMisc_cfg.Ln_r_alos_en);
        break;

    case eALL_CFG:
        dump_sd_cfg(cfg, eTX_EQ_CFG);
        dump_sd_cfg(cfg, eCDR_CFG);
        dump_sd_cfg(cfg, eSPEED_CHANGE_CFG);
        dump_sd_cfg(cfg, eRX_EQ_CFG);
        dump_sd_cfg(cfg, eDFE_CFG);
        dump_sd_cfg(cfg, ePOL_SQ_CFG);
        dump_sd_cfg(cfg, eTX_SWING_CFG);
        dump_sd_cfg(cfg, eMISC_CFG);
        break;

    case eUNKNOWN_CFG:
    default:
        break;
    }
}

static int write_serdes_config(const char *filename, const __SERDES_CONFIG_T *cfg) {
    FILE *f = fopen(filename, "w");
    if (!f) return -1;

    // tx_eq_cfg
    fprintf(f, "sTx_eq_cfg.Tap_dly=0x%x\n", cfg->sTx_eq_cfg.Tap_dly);
    fprintf(f, "sTx_eq_cfg.Tap_main=0x%x\n", cfg->sTx_eq_cfg.Tap_main);
    fprintf(f, "sTx_eq_cfg.Tap_adv=0x%x\n", cfg->sTx_eq_cfg.Tap_adv);
    fprintf(f, "sTx_eq_cfg.En_main=0x%x\n", cfg->sTx_eq_cfg.En_main);
    fprintf(f, "sTx_eq_cfg.En_adv=0x%x\n", cfg->sTx_eq_cfg.En_adv);
    fprintf(f, "sTx_eq_cfg.En_dly=0x%x\n", cfg->sTx_eq_cfg.En_dly);

    // cdr_cfg
    fprintf(f, "sCdr_cfg.Cdr_m=0x%x\n", cfg->sCdr_cfg.Cdr_m);
    fprintf(f, "sCdr_cfg.Alos_thr=0x%x\n", cfg->sCdr_cfg.Alos_thr);

    // speed_change_cfg
    fprintf(f, "sSpeed_change_cfg.L0_cfg_tx_reserve_15_8=0x%x\n", cfg->sSpeed_change_cfg.L0_cfg_tx_reserve_15_8);
    fprintf(f, "sSpeed_change_cfg.L0_cfg_tx_reserve_7_0=0x%x\n", cfg->sSpeed_change_cfg.L0_cfg_tx_reserve_7_0);
    fprintf(f, "sSpeed_change_cfg.Ln_cfg_tx_reserve_15_8=0x%x\n", cfg->sSpeed_change_cfg.Ln_cfg_tx_reserve_15_8);
    fprintf(f, "sSpeed_change_cfg.Ln_cfg_tx_reserve_7_0=0x%x\n", cfg->sSpeed_change_cfg.Ln_cfg_tx_reserve_7_0);

    // rx_eq_cfg
    fprintf(f, "sRx_eq_cfg.Ln_cfg_vga_ctrl_byp=0x%x\n", cfg->sRx_eq_cfg.Ln_cfg_vga_ctrl_byp);
    fprintf(f, "sRx_eq_cfg.Ln_cfg_vga_byp=0x%x\n", cfg->sRx_eq_cfg.Ln_cfg_vga_byp);
    fprintf(f, "sRx_eq_cfg.Ln_cfg_eqr_force=0x%x\n", cfg->sRx_eq_cfg.Ln_cfg_eqr_force);
    fprintf(f, "sRx_eq_cfg.Ln_cfg_eqc_force=0x%x\n", cfg->sRx_eq_cfg.Ln_cfg_eqc_force);

    // dfe_cfg
    fprintf(f, "sDfe_cfg.Ln_cfg_dfeck_en=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfeck_en);
    fprintf(f, "sDfe_cfg.Ln_cfg_dfe_pd=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfe_pd);
    fprintf(f, "sDfe_cfg.Ln_cfg_dfemx_pd=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfemx_pd);
    fprintf(f, "sDfe_cfg.Ln_cfg_dfe_dmux_pd=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfe_dmux_pd);
    fprintf(f, "sDfe_cfg.Ln_cfg_pi_dfe_en=0x%x\n", cfg->sDfe_cfg.Ln_cfg_pi_dfe_en);
    fprintf(f, "sDfe_cfg.Ln_cfg_dfedig_m=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfedig_m);
    fprintf(f, "sDfe_cfg.Ln_cfg_dfetap_en=0x%x\n", cfg->sDfe_cfg.Ln_cfg_dfetap_en);
    fprintf(f, "sDfe_cfg.Ln_cfg_en_dfedig=0x%x\n", cfg->sDfe_cfg.Ln_cfg_en_dfedig);

    // pol_sq_cfg
    fprintf(f, "sPol_sq_cfg.Tx_pol_inv=0x%x\n", cfg->sPol_sq_cfg.Tx_pol_inv);
    fprintf(f, "sPol_sq_cfg.Rx_pol_inv=0x%x\n", cfg->sPol_sq_cfg.Rx_pol_inv);
    fprintf(f, "sPol_sq_cfg.Ln_cfg_dis_sq=0x%x\n", cfg->sPol_sq_cfg.Ln_cfg_dis_sq);
    fprintf(f, "sPol_sq_cfg.Ln_cfg_pd_sq=0x%x\n", cfg->sPol_sq_cfg.Ln_cfg_pd_sq);

    // tx_swing_cfg
    fprintf(f, "sTx_swing_cfg.Itx_ipdriver_base=0x%x\n", cfg->sTx_swing_cfg.Itx_ipdriver_base);

    // misc_cfg
    fprintf(f, "sMisc_cfg.Cfg_common_reserve=0x%x\n", cfg->sMisc_cfg.Cfg_common_reserve);
    fprintf(f, "sMisc_cfg.Cfg_jc_byp=0x%x\n", cfg->sMisc_cfg.Cfg_jc_byp);
    fprintf(f, "sMisc_cfg.Cfg_pll_lol_set=0x%x\n", cfg->sMisc_cfg.Cfg_pll_lol_set);
    fprintf(f, "sMisc_cfg.Cfg_en_dummy=0x%x\n", cfg->sMisc_cfg.Cfg_en_dummy);
    fprintf(f, "sMisc_cfg.Cfg_pll_reserve=0x%x\n", cfg->sMisc_cfg.Cfg_pll_reserve);
    fprintf(f, "sMisc_cfg.R_dwidthctrl_from_hwt=0x%x\n", cfg->sMisc_cfg.R_dwidthctrl_from_hwt);
    fprintf(f, "sMisc_cfg.R_reg_manual=0x%x\n", cfg->sMisc_cfg.R_reg_manual);
    fprintf(f, "sMisc_cfg.Vco_div_mode=0x%x\n", cfg->sMisc_cfg.Vco_div_mode);
    fprintf(f, "sMisc_cfg.Pre_divsel=0x%x\n", cfg->sMisc_cfg.Pre_divsel);
    fprintf(f, "sMisc_cfg.Cfg_seldiv=0x%x\n", cfg->sMisc_cfg.Cfg_seldiv);
    fprintf(f, "sMisc_cfg.Data_width_sel=0x%x\n", cfg->sMisc_cfg.Data_width_sel);
    fprintf(f, "sMisc_cfg.Txfifo_ck_div=0x%x\n", cfg->sMisc_cfg.Txfifo_ck_div);
    fprintf(f, "sMisc_cfg.Rxfifo_ck_div=0x%x\n", cfg->sMisc_cfg.Rxfifo_ck_div);
    fprintf(f, "sMisc_cfg.Pma_txck_sel=0x%x\n", cfg->sMisc_cfg.Pma_txck_sel);
    fprintf(f, "sMisc_cfg.Tx_prediv=0x%x\n", cfg->sMisc_cfg.Tx_prediv);
    fprintf(f, "sMisc_cfg.Rxdiv_sel=0x%x\n", cfg->sMisc_cfg.Rxdiv_sel);
    fprintf(f, "sMisc_cfg.Txrate_sel=0x%x\n", cfg->sMisc_cfg.Txrate_sel);
    fprintf(f, "sMisc_cfg.Rxrate_sel=0x%x\n", cfg->sMisc_cfg.Rxrate_sel);
    fprintf(f, "sMisc_cfg.Ln_cfg_cdrck_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_cdrck_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_dmux_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_dmux_pd);
    fprintf(f, "sMisc_cfg.Ln_cfg_dmux_clk_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_dmux_clk_pd);
    fprintf(f, "sMisc_cfg.Ln_cfg_erramp_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_erramp_pd);
    fprintf(f, "sMisc_cfg.Ln_cfg_pi_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pi_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_pd_ctle=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pd_ctle);
    fprintf(f, "sMisc_cfg.Ln_cfg_summer_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_summer_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_pmad_ck_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pmad_ck_pd);
    fprintf(f, "sMisc_cfg.Ln_cfg_pd_clk=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pd_clk);
    fprintf(f, "sMisc_cfg.Ln_cfg_pd_cml=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pd_cml);
    fprintf(f, "sMisc_cfg.Ln_cfg_pd_driver=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pd_driver);
    fprintf(f, "sMisc_cfg.Ln_cfg_rx_reg_pu=0x%x\n", cfg->sMisc_cfg.Ln_cfg_rx_reg_pu);
    fprintf(f, "sMisc_cfg.Ln_cfg_pd_rms_det=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pd_rms_det);
    fprintf(f, "sMisc_cfg.Ln_cfg_dcdr_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_dcdr_pd);
    fprintf(f, "sMisc_cfg.Ln_cfg_ecdr_pd=0x%x\n", cfg->sMisc_cfg.Ln_cfg_ecdr_pd);
    fprintf(f, "sMisc_cfg.L0_cfg_bw=0x%x\n", cfg->sMisc_cfg.L0_cfg_bw);
    fprintf(f, "sMisc_cfg.L0_cfg_txcal_en=0x%x\n", cfg->sMisc_cfg.L0_cfg_txcal_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_bw=0x%x\n", cfg->sMisc_cfg.Ln_cfg_bw);
    fprintf(f, "sMisc_cfg.Ln_cfg_txcal_man_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_txcal_man_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_phase_man=0x%x\n", cfg->sMisc_cfg.Ln_cfg_phase_man);
    fprintf(f, "sMisc_cfg.Ln_cfg_quad_man=0x%x\n", cfg->sMisc_cfg.Ln_cfg_quad_man);
    fprintf(f, "sMisc_cfg.Ln_cfg_txcal_shift_code=0x%x\n", cfg->sMisc_cfg.Ln_cfg_txcal_shift_code);
    fprintf(f, "sMisc_cfg.Ln_cfg_txcal_valid_sel=0x%x\n", cfg->sMisc_cfg.Ln_cfg_txcal_valid_sel);
    fprintf(f, "sMisc_cfg.Ln_cfg_cdr_kf=0x%x\n", cfg->sMisc_cfg.Ln_cfg_cdr_kf);
    fprintf(f, "sMisc_cfg.Ln_cfg_pi_bw=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pi_bw);
    fprintf(f, "sMisc_cfg.Ln_cfg_pi_steps=0x%x\n", cfg->sMisc_cfg.Ln_cfg_pi_steps);
    fprintf(f, "sMisc_cfg.Ln_cfg_dis_2ndorder=0x%x\n", cfg->sMisc_cfg.Ln_cfg_dis_2ndorder);
    fprintf(f, "sMisc_cfg.Ln_cfg_rx_reserve_7_0=0x%x\n", cfg->sMisc_cfg.Ln_cfg_rx_reserve_7_0);
    fprintf(f, "sMisc_cfg.Ln_cfg_rx_reserve_15_8=0x%x\n", cfg->sMisc_cfg.Ln_cfg_rx_reserve_15_8);
    fprintf(f, "sMisc_cfg.Ln_cfg_rx_term=0x%x\n", cfg->sMisc_cfg.Ln_cfg_rx_term);
    fprintf(f, "sMisc_cfg.Ln_cfg_rx_sp_ctle=0x%x\n", cfg->sMisc_cfg.Ln_cfg_rx_sp_ctle);
    fprintf(f, "sMisc_cfg.Ln_cfg_isel_ctle=0x%x\n", cfg->sMisc_cfg.Ln_cfg_isel_ctle);
    fprintf(f, "sMisc_cfg.Ln_cfg_eqr_byp=0x%x\n", cfg->sMisc_cfg.Ln_cfg_eqr_byp);
    fprintf(f, "sMisc_cfg.Ln_cfg_agc_adpt_byp=0x%x\n", cfg->sMisc_cfg.Ln_cfg_agc_adpt_byp);
    fprintf(f, "sMisc_cfg.Ln_cfg_sum_setcm_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_sum_setcm_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_init_pos_iscan=0x%x\n", cfg->sMisc_cfg.Ln_cfg_init_pos_iscan);
    fprintf(f, "sMisc_cfg.Ln_cfg_init_pos_iPI=0x%x\n", cfg->sMisc_cfg.Ln_cfg_init_pos_iPI);
    fprintf(f, "sMisc_cfg.Cfg_i_vco=0x%x\n", cfg->sMisc_cfg.Cfg_i_vco);
    fprintf(f, "sMisc_cfg.Icp_base_sel=0x%x\n", cfg->sMisc_cfg.Icp_base_sel);
    fprintf(f, "sMisc_cfg.Icp_sel=0x%x\n", cfg->sMisc_cfg.Icp_sel);
    fprintf(f, "sMisc_cfg.Cfg_rsel=0x%x\n", cfg->sMisc_cfg.Cfg_rsel);
    fprintf(f, "sMisc_cfg.Ln_cfg_iscan_en=0x%x\n", cfg->sMisc_cfg.Ln_cfg_iscan_en);
    fprintf(f, "sMisc_cfg.Ln_cfg_en_fast_iscan=0x%x\n", cfg->sMisc_cfg.Ln_cfg_en_fast_iscan);
    fprintf(f, "sMisc_cfg.Ln_cfg_filter2nd_yz_6_0=0x%x\n", cfg->sMisc_cfg.Ln_cfg_filter2nd_yz_6_0);
    fprintf(f, "sMisc_cfg.Ln_r_alos_en=0x%x\n", cfg->sMisc_cfg.Ln_r_alos_en);

    fclose(f);
    return 0;
}

// Read config from file in Tag=Value format
static int read_serdes_config(const char *filename, __SERDES_CONFIG_T *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    char line[128];
    char tag[64];
    unsigned int value;
    while (fgets(line, sizeof(line), f)) {
        // Accept both 0x and decimal, but expect 0x
        if (sscanf(line, "%63[^=]=0x%x", tag, &value) != 2)
            continue;

        // tx_eq_cfg
        if (strcmp(tag, "sTx_eq_cfg.Tap_dly") == 0) cfg->sTx_eq_cfg.Tap_dly = (u8)value;
        else if (strcmp(tag, "sTx_eq_cfg.Tap_main") == 0) cfg->sTx_eq_cfg.Tap_main = (u8)value;
        else if (strcmp(tag, "sTx_eq_cfg.Tap_adv") == 0) cfg->sTx_eq_cfg.Tap_adv = (u8)value;
        else if (strcmp(tag, "sTx_eq_cfg.En_main") == 0) cfg->sTx_eq_cfg.En_main = (u8)value;
        else if (strcmp(tag, "sTx_eq_cfg.En_adv") == 0) cfg->sTx_eq_cfg.En_adv = (u8)value;
        else if (strcmp(tag, "sTx_eq_cfg.En_dly") == 0) cfg->sTx_eq_cfg.En_dly = (u8)value;

        // cdr_cfg
        else if (strcmp(tag, "sCdr_cfg.Cdr_m") == 0) cfg->sCdr_cfg.Cdr_m = (u8)value;
        else if (strcmp(tag, "sCdr_cfg.Alos_thr") == 0) cfg->sCdr_cfg.Alos_thr = (u8)value;

        // speed_change_cfg
        else if (strcmp(tag, "sSpeed_change_cfg.L0_cfg_tx_reserve_15_8") == 0) cfg->sSpeed_change_cfg.L0_cfg_tx_reserve_15_8 = (u8)value;
        else if (strcmp(tag, "sSpeed_change_cfg.L0_cfg_tx_reserve_7_0") == 0) cfg->sSpeed_change_cfg.L0_cfg_tx_reserve_7_0 = (u8)value;
        else if (strcmp(tag, "sSpeed_change_cfg.Ln_cfg_tx_reserve_15_8") == 0) cfg->sSpeed_change_cfg.Ln_cfg_tx_reserve_15_8 = (u8)value;
        else if (strcmp(tag, "sSpeed_change_cfg.Ln_cfg_tx_reserve_7_0") == 0) cfg->sSpeed_change_cfg.Ln_cfg_tx_reserve_7_0 = (u8)value;

        // rx_eq_cfg
        else if (strcmp(tag, "sRx_eq_cfg.Ln_cfg_vga_ctrl_byp") == 0) cfg->sRx_eq_cfg.Ln_cfg_vga_ctrl_byp = (u8)value;
        else if (strcmp(tag, "sRx_eq_cfg.Ln_cfg_vga_byp") == 0) cfg->sRx_eq_cfg.Ln_cfg_vga_byp = (u8)value;
        else if (strcmp(tag, "sRx_eq_cfg.Ln_cfg_eqr_force") == 0) cfg->sRx_eq_cfg.Ln_cfg_eqr_force = (u8)value;
        else if (strcmp(tag, "sRx_eq_cfg.Ln_cfg_eqc_force") == 0) cfg->sRx_eq_cfg.Ln_cfg_eqc_force = (u8)value;

        // dfe_cfg
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfeck_en") == 0) cfg->sDfe_cfg.Ln_cfg_dfeck_en = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfe_pd") == 0) cfg->sDfe_cfg.Ln_cfg_dfe_pd = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfemx_pd") == 0) cfg->sDfe_cfg.Ln_cfg_dfemx_pd = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfe_dmux_pd") == 0) cfg->sDfe_cfg.Ln_cfg_dfe_dmux_pd = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_pi_dfe_en") == 0) cfg->sDfe_cfg.Ln_cfg_pi_dfe_en = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfedig_m") == 0) cfg->sDfe_cfg.Ln_cfg_dfedig_m = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_dfetap_en") == 0) cfg->sDfe_cfg.Ln_cfg_dfetap_en = (u8)value;
        else if (strcmp(tag, "sDfe_cfg.Ln_cfg_en_dfedig") == 0) cfg->sDfe_cfg.Ln_cfg_en_dfedig = (u8)value;

        // pol_sq_cfg
        else if (strcmp(tag, "sPol_sq_cfg.Tx_pol_inv") == 0) cfg->sPol_sq_cfg.Tx_pol_inv = (u8)value;
        else if (strcmp(tag, "sPol_sq_cfg.Rx_pol_inv") == 0) cfg->sPol_sq_cfg.Rx_pol_inv = (u8)value;
        else if (strcmp(tag, "sPol_sq_cfg.Ln_cfg_dis_sq") == 0) cfg->sPol_sq_cfg.Ln_cfg_dis_sq = (u8)value;
        else if (strcmp(tag, "sPol_sq_cfg.Ln_cfg_pd_sq") == 0) cfg->sPol_sq_cfg.Ln_cfg_pd_sq = (u8)value;

        // tx_swing_cfg
        else if (strcmp(tag, "sTx_swing_cfg.Itx_ipdriver_base") == 0) cfg->sTx_swing_cfg.Itx_ipdriver_base = (u8)value;

        // misc_cfg
        else if (strcmp(tag, "sMisc_cfg.Cfg_common_reserve") == 0) cfg->sMisc_cfg.Cfg_common_reserve = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_jc_byp") == 0) cfg->sMisc_cfg.Cfg_jc_byp = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_pll_lol_set") == 0) cfg->sMisc_cfg.Cfg_pll_lol_set = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_en_dummy") == 0) cfg->sMisc_cfg.Cfg_en_dummy = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_pll_reserve") == 0) cfg->sMisc_cfg.Cfg_pll_reserve = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.R_dwidthctrl_from_hwt") == 0) cfg->sMisc_cfg.R_dwidthctrl_from_hwt = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.R_reg_manual") == 0) cfg->sMisc_cfg.R_reg_manual = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Vco_div_mode") == 0) cfg->sMisc_cfg.Vco_div_mode = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Pre_divsel") == 0) cfg->sMisc_cfg.Pre_divsel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_seldiv") == 0) cfg->sMisc_cfg.Cfg_seldiv = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Data_width_sel") == 0) cfg->sMisc_cfg.Data_width_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Txfifo_ck_div") == 0) cfg->sMisc_cfg.Txfifo_ck_div = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Rxfifo_ck_div") == 0) cfg->sMisc_cfg.Rxfifo_ck_div = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Pma_txck_sel") == 0) cfg->sMisc_cfg.Pma_txck_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Tx_prediv") == 0) cfg->sMisc_cfg.Tx_prediv = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Rxdiv_sel") == 0) cfg->sMisc_cfg.Rxdiv_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Txrate_sel") == 0) cfg->sMisc_cfg.Txrate_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Rxrate_sel") == 0) cfg->sMisc_cfg.Rxrate_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_cdrck_en") == 0) cfg->sMisc_cfg.Ln_cfg_cdrck_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_dmux_pd") == 0) cfg->sMisc_cfg.Ln_cfg_dmux_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_dmux_clk_pd") == 0) cfg->sMisc_cfg.Ln_cfg_dmux_clk_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_erramp_pd") == 0) cfg->sMisc_cfg.Ln_cfg_erramp_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pi_en") == 0) cfg->sMisc_cfg.Ln_cfg_pi_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pd_ctle") == 0) cfg->sMisc_cfg.Ln_cfg_pd_ctle = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_summer_en") == 0) cfg->sMisc_cfg.Ln_cfg_summer_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pmad_ck_pd") == 0) cfg->sMisc_cfg.Ln_cfg_pmad_ck_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pd_clk") == 0) cfg->sMisc_cfg.Ln_cfg_pd_clk = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pd_cml") == 0) cfg->sMisc_cfg.Ln_cfg_pd_cml = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pd_driver") == 0) cfg->sMisc_cfg.Ln_cfg_pd_driver = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_rx_reg_pu") == 0) cfg->sMisc_cfg.Ln_cfg_rx_reg_pu = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pd_rms_det") == 0) cfg->sMisc_cfg.Ln_cfg_pd_rms_det = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_dcdr_pd") == 0) cfg->sMisc_cfg.Ln_cfg_dcdr_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_ecdr_pd") == 0) cfg->sMisc_cfg.Ln_cfg_ecdr_pd = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.L0_cfg_bw") == 0) cfg->sMisc_cfg.L0_cfg_bw = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.L0_cfg_txcal_en") == 0) cfg->sMisc_cfg.L0_cfg_txcal_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_bw") == 0) cfg->sMisc_cfg.Ln_cfg_bw = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_txcal_man_en") == 0) cfg->sMisc_cfg.Ln_cfg_txcal_man_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_phase_man") == 0) cfg->sMisc_cfg.Ln_cfg_phase_man = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_quad_man") == 0) cfg->sMisc_cfg.Ln_cfg_quad_man = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_txcal_shift_code") == 0) cfg->sMisc_cfg.Ln_cfg_txcal_shift_code = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_txcal_valid_sel") == 0) cfg->sMisc_cfg.Ln_cfg_txcal_valid_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_cdr_kf") == 0) cfg->sMisc_cfg.Ln_cfg_cdr_kf = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pi_bw") == 0) cfg->sMisc_cfg.Ln_cfg_pi_bw = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_pi_steps") == 0) cfg->sMisc_cfg.Ln_cfg_pi_steps = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_dis_2ndorder") == 0) cfg->sMisc_cfg.Ln_cfg_dis_2ndorder = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_rx_reserve_7_0") == 0) cfg->sMisc_cfg.Ln_cfg_rx_reserve_7_0 = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_rx_reserve_15_8") == 0) cfg->sMisc_cfg.Ln_cfg_rx_reserve_15_8 = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_rx_term") == 0) cfg->sMisc_cfg.Ln_cfg_rx_term = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_rx_sp_ctle") == 0) cfg->sMisc_cfg.Ln_cfg_rx_sp_ctle = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_isel_ctle") == 0) cfg->sMisc_cfg.Ln_cfg_isel_ctle = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_eqr_byp") == 0) cfg->sMisc_cfg.Ln_cfg_eqr_byp = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_agc_adpt_byp") == 0) cfg->sMisc_cfg.Ln_cfg_agc_adpt_byp = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_sum_setcm_en") == 0) cfg->sMisc_cfg.Ln_cfg_sum_setcm_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_init_pos_iscan") == 0) cfg->sMisc_cfg.Ln_cfg_init_pos_iscan = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_init_pos_iPI") == 0) cfg->sMisc_cfg.Ln_cfg_init_pos_iPI = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_i_vco") == 0) cfg->sMisc_cfg.Cfg_i_vco = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Icp_base_sel") == 0) cfg->sMisc_cfg.Icp_base_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Icp_sel") == 0) cfg->sMisc_cfg.Icp_sel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Cfg_rsel") == 0) cfg->sMisc_cfg.Cfg_rsel = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_iscan_en") == 0) cfg->sMisc_cfg.Ln_cfg_iscan_en = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_en_fast_iscan") == 0) cfg->sMisc_cfg.Ln_cfg_en_fast_iscan = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_cfg_filter2nd_yz_6_0") == 0) cfg->sMisc_cfg.Ln_cfg_filter2nd_yz_6_0 = (u8)value;
        else if (strcmp(tag, "sMisc_cfg.Ln_r_alos_en") == 0) cfg->sMisc_cfg.Ln_r_alos_en = (u8)value;
    }
    fclose(f);
    dump_sd_cfg(*cfg, eALL_CFG);
    return 0;
}

static void cli_cmd_phy_serdes_get(cli_req_t *req)
{
    mepa_rc rc;
    SD_CFG_SPEED_IDX_t speed;
    __SERDES_CONFIG_T sd_cfg = {0};
    phy_sd_cli_req_t *mreq = req->module_req;

    for (speed = 0; speed < SD_UNKNOWN_SPEED; speed++) {
        if (speed != mreq->speed_idx)
        {
            continue;
        }
        printf ("SERDES CONFIG GET - %d\n", speed);
        rc = lan80xx_get_serdes_config(meba_phy_kr_inst->phy_devices[0], speed, eALL_CFG, &sd_cfg);
        if (rc != MESA_RC_OK) {
            T_E(MEPA_TRACE_GRP_GEN, "serdes config get failed\n");
            break;
        }
        printf ("#####################\n");
        dump_sd_cfg(sd_cfg, eALL_CFG);
        write_serdes_config("/tmp/serdes_rd.cfg", &sd_cfg);
    }
}

static void cli_cmd_phy_serdes_set(cli_req_t *req)
{
    mepa_rc rc;
    SD_CFG_SPEED_IDX_t speed;
    __SERDES_CONFIG_T cfg = {0};
    phy_sd_cli_req_t *mreq = req->module_req;

    if (read_serdes_config("/tmp/serdes_wr.cfg", &cfg) != 0)
    {
        printf ("File (%s) does not exist!! Create a file with presets to load\n", "/tmp/serdes_wr.cfg");
        return;
    }

    for (speed = 0; speed < SD_UNKNOWN_SPEED; speed++) {
        if (speed != mreq->speed_idx)
        {
            continue;
        }
        rc = lan80xx_set_serdes_config(meba_phy_kr_inst->phy_devices[0], speed, eALL_CFG, &cfg);
        if (rc != MESA_RC_OK) {
            T_E(MEPA_TRACE_GRP_GEN, "serdes config set failed\n");
            break;
        }
    }
}

static void cli_cmd_phy_kr(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family = {0};
    phy_kr_cli_req_t *mreq = req->module_req;
    mepa_port_no_t  port_no;
    mepa_conf_t conf = {0};
    const uint8_t u8linkup_time = 3;
    uint8_t LinkSpeed = 0;

    /* loop till phy_device_cnt - 1, as last port is management port */
    for (int iport = 0; iport < meba_phy_kr_inst->phy_device_cnt - 1; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if (meba_phy_kr_inst->phy_devices[iport] == NULL) {
            continue;
        }
        if ((rc = phy_family_detect(meba_phy_kr_inst, iport, &phy_family)) != MEPA_RC_OK) {
            cli_printf ("\nError in Detecting PHY Family on Port %d\n", iport);
            continue;
        }
        if (req->set) {
            if (phy_family.family == PHY_FAMILY_MALIBU_25G) {
                /* Get old config to retain mepa related context */
                if ((rc = mepa_conf_get(meba_phy_kr_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
                    T_E("\n mepa_conf_get failed on port %d\n", iport);
                    continue;
                }
                if (mreq->dis == TRUE) {
                    conf.speed = MESA_SPEED_AUTO;
                    /* with out aneg, no training */
                    conf.conf_25g.kr_train_enable = 0;
                    conf.adv_dis = TRUE;
                } else {
                    conf.adv_dis = FALSE;
                    conf.speed = MESA_SPEED_AUTO;
                    conf.conf_25g.kr_train_enable = mreq->train;
                    conf.aneg.speed_10g_fdx = mreq->adv10g;
                    conf.aneg.speed_1g_fdx = mreq->adv1g;
                    conf.aneg.speed_25g_fdx = mreq->adv25g_kr;
                    conf.aneg.speed_25g_kr_s_fdx = mreq->adv25g_krs;
                    conf.aneg.next_page_enable = mreq->np;
                    conf.aneg.advertise_dir = MEPA_ADV_SIDE_LINE;
                    conf.conf_25g.base_r_10gfec = mreq->rfec_10g;
                    conf.conf_25g.base_r_25gfec = mreq->rfec_25g;
                    conf.conf_25g.rs_fec_25g = mreq->rsfec_25g;
                    conf.conf_25g.np_base_r_fec = mreq->np_rfec;
                    conf.conf_25g.np_rs_fec = mreq->np_rsfec;
                    conf.conf_25g.fw_resolve = mreq->fw_res;
                }
                /* set new config */
                if ((rc = mepa_conf_set(meba_phy_kr_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
                    T_E("\n mepa_conf_set failed on port %d\n", iport);
                    continue;
                }
            }
        } else {
            if ((rc = mepa_conf_get(meba_phy_kr_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
                T_E("\n mepa_conf_get failed on port %d\n", iport);
                continue;
            }
            cli_printf("Port: %d\n", iport);
            cli_printf("  KR aneg: %s\n", (conf.speed == MESA_SPEED_AUTO) ? "Enabled" : "Disabled");
            cli_printf("  KR training: %s\n", conf.conf_25g.kr_train_enable ? "Enabled" : "Disabled");
        }
    }
    /*
     * The below logic is specific to M25G
     * line side ports configured with the requested cofnig
     * then wait for 3sec (shall be modified through command)
     * check the linkup status on LINE SIDE ports
     * Link up - get (LINE linkup speed) and configure HOST side
     * Link fail - disable aneg
     */
    if (req->set && (phy_family.family == PHY_FAMILY_MALIBU_25G)) {
        /* Wait for the LINE link up */
        sleep(u8linkup_time);
        for (int iport = 0; iport < meba_phy_kr_inst->phy_device_cnt - 1; iport++) {
            port_no = iport2uport(iport);
            if (req->port_list[port_no] == 0) {
                continue;
            }
            LinkSpeed = MESA_SPEED_UNDEFINED;
            /* check the linkup state for line side */
            rc = Islinkup(iport, MEPA_ADV_SIDE_LINE, &LinkSpeed);
            /* Get the link up speed */
            if (rc == MESA_RC_OK && mreq->dis == FALSE) {
                if (LinkSpeed == 7) {
                    cli_printf("Line side link up speed: 25G kr\n");
                }
                if (LinkSpeed == 8) {
                    cli_printf("Line side link up speed: 25G kr-s\n");
                }
                if (LinkSpeed == 9) {
                    cli_printf("Line side link up speed: 10G\n");
                }
                if (LinkSpeed == 13) {
                    cli_printf("Line side link up speed: 1G\n");
                }
                if ((rc = mepa_conf_get(meba_phy_kr_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
                    T_E("\n mepa_conf_get failed on port %d\n", iport);
                    continue;
                }
                /* Send the command to M25g for host side */
                conf.adv_dis = FALSE;
                conf.speed = MESA_SPEED_AUTO;
                conf.aneg.speed_25g_fdx = (LinkSpeed == 7) ? 1 : 0;
                conf.aneg.speed_25g_kr_s_fdx = (LinkSpeed == 8) ? 1 : 0;
                conf.aneg.speed_10g_fdx = (LinkSpeed == 9) ? 1 : 0;
                conf.aneg.speed_1g_fdx = (LinkSpeed == 13) ? 1 : 0;
                conf.aneg.advertise_dir = MEPA_ADV_SIDE_HOST;
            } else {
                /* Configure fixed 25g speed */
                conf.speed = MESA_SPEED_AUTO;
                conf.conf_25g.kr_train_enable = 0;
                conf.adv_dis = TRUE;
            }

            if ((rc = mepa_conf_set(meba_phy_kr_inst->phy_devices[iport], &conf)) != MESA_RC_OK) {
                T_E("\n mepa_conf_set failed on port %d\n", iport);
                continue;
            }
            /*
             * Return from here!!
             * don't wait for Host side link up
             * As the HOST is connected to EDSX serdes
             * EDSX serdes must run ANEG
             * So, return from here, so EDSx AN SM starts
             */
        }
    }
    return;
}

////////////////////////////////////////////////////////////
/////////////////   CLI Parameter functions ////////////////
////////////////////////////////////////////////////////////

static int cli_kr_parm_keyword(cli_req_t *req)
{
    const char     *found;
    phy_kr_cli_req_t *mreq = req->module_req;

    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        return 1;
    }

    //cli_printf ("M25G KR ANEG cli param\n");
    if (!strncasecmp(found, "adv-1g", 6)) {
        mreq->adv1g = 1;
    } else if (!strncasecmp(found, "adv-10g", 7)) {
        mreq->adv10g = 1;
    } else if (!strncasecmp(found, "adv-25g", 7)) {
        mreq->adv25g_kr = 1;
    } else if (!strncasecmp(found, "adv-25g-krs", 11)) {
        mreq->adv25g_krs = 1;
    } else if (!strncasecmp(found, "rfec-10g", 8)) {
        mreq->rfec_10g = 1;
    } else if (!strncasecmp(found, "rfec-25g", 8)) {
        mreq->rfec_25g = 1;
    } else if (!strncasecmp(found, "rsfec", 5)) {
        mreq->rsfec_25g = 1;
    } else if (!strncasecmp(found, "np", 2)) {
        mreq->np = 1;
    } else if (!strncasecmp(found, "np-rfec", 7)) {
        mreq->np_rfec = 1;
    } else if (!strncasecmp(found, "np-rsfec", 8)) {
        mreq->np_rsfec = 1;
    } else if (!strncasecmp(found, "train", 5)) {
        mreq->train = 1;
    } else if (!strncasecmp(found, "disable", 7)) {
        mreq->dis = 1;
    } else if (!strncasecmp(found, "fw-res", 6)) {
        mreq->fw_res = 1;
    } else {
        cli_printf("no match: %s\n", found);
    }

    return 0;
}

static int cli_parm_sd_speed_idx (cli_req_t *req)
{
    phy_sd_cli_req_t *mreq = req->module_req;

    return cli_parm_u8(req, &mreq->speed_idx, 0, 3);
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "Phy KR aneg [<port_list>] [adv-1g] [adv-10g] [adv-25g] [adv-25g-krs] [rfec-10g] [rfec-25g] [rsfec] [np] [np-rfec] [np-rsfec] [fw-res] [train] [disable]",
        "Set or show kr",
        cli_cmd_phy_kr
    },
    {
        "Phy serdes get <speed_idx>",
        "Get phy serdes configuration",
        cli_cmd_phy_serdes_get
    },
    {
        "Phy serdes set <speed_idx>",
        "Set phy serdes configuration",
        cli_cmd_phy_serdes_set
    },
    /* {
         "Phy KR status [<port_list>] [eq] [ber] [irq] [all] [clr]",
         "Show status",
         cli_cmd_phy_kr_status
     },*/
};

static cli_parm_t cli_parm_table[] = {
    {
        "adv-1g",
        "adv-1g: advertise 1g",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "adv-10g",
        "adv-10g: advertise 10g",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "adv-25g",
        "adv-25g: advertise 25g kr",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "adv-25g-krs",
        "adv-25g-krs: advertise 25g krs",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },

    {
        "rfec-10g",
        "rfec: advertise 10g r-fec capability",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "rfec-25g",
        "rfec: advertise 25g r-fec capability",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },

    {
        "rsfec",
        "rs-fec: advertise 25g rs-fec capability",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "np",
        "np: use next page for advertise",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "np-rfec",
        "np r-fec: advertise next page r-fec capability",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "np-rsfec",
        "np rs-fec: advertise next page rs-fec capability",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "fw-res",
        "fw-res: enable firmware resolve",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },

    {
        "train",
        "train: enable training",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "disable",
        "disable: disable kr",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_kr_parm_keyword,
        cli_cmd_phy_kr
    },
    {
        "<speed_idx>",
        "speed_idx: Speed index 0 - 1G, 1 - 10G, 2 - 25G",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_sd_speed_idx,
    },


};

////////////////////////////////////////////////////////////
/////////////// Core init functions ////////////////////////
////////////////////////////////////////////////////////////

static int phy_kr_cli_init(void)
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
    return MESA_RC_OK;
}

void mscc_appl_phy_kr_init(mscc_appl_init_t *init)
{
    meba_phy_kr_inst = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        if (phy_kr_cli_init() != MEPA_RC_OK) {
            T_E("Couldn't load phy kr aneg config module");
        }
        break;
    default:
        break;
    }
}
