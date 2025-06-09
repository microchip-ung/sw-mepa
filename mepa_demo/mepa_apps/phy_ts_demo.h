// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


/*============================================================
 * Keywords used in Time Stampping Commands                          |
 *============================================================
*/

#define KEYWORD_CLOCK_SOURCE        "clk_src"   // Keyword used in command ts_init_conf
#define KEYWORD_TX_FIFO_MODE        "tx_fifo"   // Keyword used in command ts_init_conf   
#define KEYWORD_TC_OP_MODE          "tc_op"     // Keyword used in command ts_init_conf 
#define KEYWORD_DLY_REQ_10B         "dly_10b"   // Keyword used in command ts_init_conf 
#define KEYWORD_TX_AUTO_FOLLOWUP    "tx_af"     // Keyword used in command ts_init_conf
#define KEYWORD_MCH_EN              "mch"       // Keyword used in command ts_init_conf


#define KEYWORD_FLOW_INDEX      "flow_idx"      // Keyword used in command tx_class_conf, rx_class_conf
#define KEYWORD_ENCAP_TYPE      "entype"        // Keyword used in command tx_class_conf, rx_class_conf

#define KEYWORD_CLOCK_ID        "clk_id"        // Keyword used in command tx_clock_conf, rx_clock_conf
#define KEYWORD_CLOCK_MODE      "clk_mode"      // Keyword used in command tx_clock_conf, rx_clock_conf
#define KEYWORD_DELAY_TYPE      "dly_type"      // Keyword used in command tx_clock_conf, rx_clock_conf

#define KEYWORD_LOAD            "load"          // Keyword used in command ts_ltc
#define KEYWORD_SAVE            "save"          // Keyword used in command ts_ltc

#define KEYWORD_ACTION            "pin_action"          // Keyword used in command ts_ltc
#define KEYWORD_LS_CTRL_SEL       "ls_ctrl_sel"          // Keyword used in command ts_ltc

#define KEYWORD_LTC_TIME        "time"

#define KEYWORD_DELAY_MODE      "timing_mode"   // Keyword used in command ts_delay_set
#define KEYWORD_DELAY           "delay"         // Keyword used in command ts_delay_set

#define KEYWORD_CONF_SEL        "conf_sel"      // Keyword used in command ts_conf_get

#define KEYWORD_RATEADJ         "rateadj"       // Keyword used in ts_clk_rateadj_set

#define KEYWORD_CLK_SEL         "clk_sel"
#define KEYWORD_PIN_SEL         "pin_sel"
#define KEYWORD_POL             "pol"
#define KEYWORD_SYNC_MODE       "sync_mode"
#define KEYWORD_NS_ENABLE       "ns_en"
#define KEYWORD_PPS_WIDTH       "pps_wid"
#define KEYWORD_PPS_INTERVAL    "pps_in"
#define KEYWORD_WFH_PERIOD      "wfh"
#define KEYWORD_WFL_PERIOD      "wfl"
#define KEYWORD_DELTA_ADJ       "adj"


typedef struct {
    mepa_bool_t     clk_src_parsed;         /* ts_init_conf - Clock source parsed */
    mepa_bool_t     tx_fifo_parsed;         /* ts_init_conf - Tx FIFO Mode parsed */
    mepa_bool_t     tc_op_parsed;           /* ts_init_conf - Tc Op Mode parsed */
    mepa_bool_t     dly_req_10b_parsed;     /* ts_init_conf - Delay request 10B parsed */
    mepa_bool_t     tx_auto_f_parsed;       /* ts_init_conf - Tx auto follow up parsed */
    mepa_bool_t     mch_en_parsed;          /* ts_init_conf - MCH parsed */
    mepa_bool_t     flow_idx_parsed;        /* tx_class_conf - Flow Index Parsed */
    mepa_bool_t     encap_type_parsed;      /* tx_class_conf - encapsulation type parsed */
    mepa_bool_t     clk_id_parsed;          /* tx_clock_conf - Clock Id parsed */
    mepa_bool_t     clk_mode_parsed;        /* tx_clock_conf - Clock Mode parsed */
    mepa_bool_t     delaym_type_parsed;     /* tx_clock_conf - Delay type parsed */
    mepa_bool_t     ltc_time_parsed;
    mepa_bool_t     sig_mask_parsed;
    mepa_bool_t     delay_mode_parsed;      /* ts_delay_set, */
    mepa_bool_t     delay_parsed;           /* ts_delay_set */
    mepa_bool_t     conf_sel_parsed;            /* ts_conf_get */
    mepa_bool_t     rateadj_parsed;         /* ts_clk_rateadj_set*/
    mepa_bool_t     action_parsed;
    mepa_bool_t     ls_ctrl_sel_parsed;
    mepa_bool_t     clk_sel_parsed;
    mepa_bool_t     pin_sel_parsed;
    mepa_bool_t     pol_parsed;
    mepa_bool_t     sync_mode_parsed;
    mepa_bool_t     ns_enable_parsed;
    mepa_bool_t     pps_width_parsed;
    mepa_bool_t     pps_interval_parsed;
    mepa_bool_t     wfh_parsed;
    mepa_bool_t     wfl_parsed;
    mepa_bool_t     delta_adj_parsed;
} ts_keyword_parsed;


typedef struct {
    uint16_t    clk_src;
    uint16_t    tx_fifo_mode;
    uint16_t    tc_op_mode;
    mepa_bool_t dly_req_10b;
    mepa_bool_t tx_auto_f;
    mepa_bool_t mch_en;
    uint16_t    flow_index;
    uint16_t    clk_id;
    uint8_t     clk_mode;
    uint8_t     delay_type;
    uint16_t    encap_type;
    uint16_t    sec_high;
    uint32_t    sec_low;
    uint32_t    nanoseconds;
    uint8_t     picoseconds;
    uint16_t    fifo_size;
    uint16_t    sig_mask;
    uint8_t     delay_mode;   /* ts_delay_set*/
    double      delay;
    uint8_t     config;
    double      rateadj_ppb;
    uint8_t     action;
    uint8_t      ls_ctrl_sel;
    uint8_t      clk_select;
    uint8_t      pin_select;
    mepa_bool_t  pin_inv_pol;
    uint8_t      pin_sync_mode;
    uint8_t      lsc_select;
    mepa_bool_t tod_load;
    mepa_bool_t tod_save;
    mepa_bool_t ns_enable;
    uint32_t    pps_width;
    uint32_t    pps_interval;
    uint32_t    wfh_period;
    uint32_t    wfl_period;
} ts_configuration;

