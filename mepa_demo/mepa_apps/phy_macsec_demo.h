// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


/*============================================================
 * Keywords used in MACsec Commands                          |
 *============================================================
*/

/* MACsec Enable or Disable Command Keywords */
#define KEYWORD_BYPASS_NONE     "bypass_none"
#define KEYWORD_BYPASS_DISABLE  "bypass_disable"
#define KEYWORD_BYPASS_ENABLE   "bypass_enable"


/* MACsec SecY Configuration keywords */
#define KEYWORD_CREATE          "create"
#define KEYWORD_UPDATE          "update"
#define KEYWORD_TRUE            "true"
#define KEYWORD_FALSE           "false"
#define KEYWORD_EGRESS          "egress"
#define KEYWORD_INGRESS         "ingress"
#define KEYWORD_CONT_PORT       "cont_port"
#define KEYWORD_UNCONT_PORT     "uncont_port"
#define KEYWORD_DROP            "drop"

/* MACsec Pattern Matching Keywords */
#define KEYWORD_ETHTYPE         "ethtype"
#define KEYWORD_SRC_MAC         "src_mac"
#define KEYWORD_DST_MAC         "dst_mac"
#define KEYWORD_VLANID          "vlanid"
#define KEYWORD_VIDINNER        "vid-inner"

/* Secure Channel Keywords */
#define KEYWORD_SC_ID           "sc-id"
#define KEYWORD_AN              "an"
#define KEYWORD_NEXT_PN         "next-pn"
#define KEYWORD_CONF            "conf"
#define KEYWORD_LOW_PN          "lowest-pn"

/* MACsec Statistics keywords */
#define KEYWORD_GET             "get"
#define KEYWORD_CLEAR           "clear"
#define KEYWORD_HMAC            "hmac"
#define KEYWORD_LMAC            "lmac"
#define KEYWORD_SET             "set"

/* MACsec VLAN Tag Keywords */
#define KEYWORD_VLAN_ZERO       "zero"
#define KEYWORD_VLAN_ONE        "one"
#define KEYWORD_VLAN_TWO        "two"
#define KEYWORD_VLAN_THREE      "three"
#define KEYWORD_VLAN_FOUR       "four"

/* MACsec Conf Get Keyword */
#define KEYWORD_SECY_GET        "secy"
#define KEYWORD_TX_SC_GET       "tx_sc"
#define KEYWORD_TX_SA_GET       "tx_sa"
#define KEYWORD_RX_SC_GET       "rx_sc"
#define KEYWORD_RX_SA_GET       "rx_sa"

/* MACsec Event Keywords */
#define KEYWORD_EVT_ENABLE      "evt_enable"
#define KEYWORD_ROLLOVER_EVT    "rollover"
#define KEYWORD_SEQ_THRE_EVT    "seq_threshold"

#define MAC_ADDRESS_LEN    17 /* Length of MAC address string in formate "XX-XX-XX-XX-XX-XX" */
#define MAX_VALUE_LIST     10

typedef struct {
    mepa_bool_t     etype_parsed;         /* Ethertype Keyword Parsed */
    mepa_bool_t     src_mac_parsed;       /* Source MAC Address keyword Parsed */
    mepa_bool_t     dst_mac_parsed;       /* Destination MAC Address keyword Parsed */
    mepa_bool_t     vlan_id_parsed;       /* Vlan id Keyword Parsed */
    mepa_bool_t     vlan_inner_id_parsed; /* Inner Vlan id keyword Parsed */
    mepa_bool_t     rx_sc_id_parsed;      /* Receive Secure Channel identifier Parsed */
    mepa_bool_t     next_pn_parsed;       /* Next packet number keyword Parsed */
    mepa_bool_t     an_parsed;            /* Association Number Keyword parsed */
    mepa_bool_t     conf_parsed;          /* Confidentiality Keyword Parsed */
    mepa_bool_t     lowest_pn_parsed;     /* Lowest Packet Number for RX keyword Parsed */
} keyword_parsed;

/* Stroring the required Parameters for the Application in structure */
typedef struct {
    mepa_etype_t     pattern_ethtype;
    mepa_mac_t       pattern_src_mac_addr;
    mepa_mac_t       pattern_dst_mac_addr;
    mepa_vid_t       vid;
    mepa_vid_t       vid_inner;
} macsec_store;

typedef struct {
    mepa_bool_t   rollover_event;      /* Rollover Event Enabled */
    mepa_bool_t   seq_thres_event;     /* Sequence Threshold Event Enabled */
    mepa_bool_t   rollover_status;     /* Rollover Event Interrupt Status */
    mepa_bool_t   seq_thres_status;    /* Sequence Threshold Event Interrupt Status */
} macsec_event_status;

typedef enum {
    MACSEC_CONF_GET_NONE,
    MACSEC_SECY_CONF_GET,            /* Get SecY Configuration */
    MACSEC_TX_SC_CONF_GET,           /* Get Transmit Secure Channel Configuration */
    MACSEC_TX_SA_CONF_GET,           /* Get Transmit Secure Association Configuration */
    MACSEC_RX_SC_CONF_GET,           /* Get Receive Secure Channel Configuration */
    MACSEC_RX_SA_CONF_GET,           /* Get Receive Secure Association Configuration */
} macsec_conf_get;

typedef struct {
    mepa_macsec_init_bypass_t   bypass_conf;          /* Bypass Configuration */
    mepa_bool_t                 protect_frames;       /* Frame Protection Enable or disable */
    mepa_validate_frames_t      frame_validate;       /* Frame Vlaidation Configuration */
    uint32_t                    port_id;              /* SecY Identifier */
    mepa_macsec_ciphersuite_t   cipher;               /* Cipher Suit Configuration */
    mepa_bool_t                 secy_create;          /* SecY Create or Update */
    mepa_bool_t                 egress_direction;     /* Egress or Ingress Direction */
    mepa_mac_t                  mac_addr;             /* SecY MAC Address */
    mepa_etype_t                pattern_ethtype;      /* Pattern match Ethertype */
    mepa_mac_t                  pattern_src_mac_addr; /* Pattern Match SRC MAC Address */
    mepa_mac_t                  pattern_dst_mac_addr; /* pattern Match DST MAC Address */
    mepa_macsec_match_action_t  match_action;         /* Action after PAttern Matched */
    macsec_conf_get             conf_get;             /* Configuration Get */
    uint16_t                    vid;                  /* Pattern Match VLAN ID */
    uint16_t                    vid_inner;            /* Pattern Match Inner VLAN ID */
    uint16_t                    an_no;                /* Association Number for SA */
    uint16_t                    rx_sc_id;             /* Receive Secure Channel identifier */
    uint64_t                    next_pn;              /* Next Packet number */
    uint64_t                    lowest_pn;            /* Lowest Packet number */
    uint64_t                    seq_threshold_value;  /* Sequence Threshold packet Number */
    uint16_t                    vlan_tag;             /* Number of VLAn Tags needs to be bypassed */
    mepa_bool_t                 conf;                 /* Confidentiality Enable */
    mepa_bool_t                 statistics_get;       /* MACsec Statistics Get or Clear */
    mepa_bool_t                 hmac_counters;        /* HMAC counters or LMAC Counters */
    mepa_bool_t                 rollover_event;       /* MACsec Rollover Event */
    mepa_bool_t                 sequence_threshold_event; /* MACsec Sequence threshold Event */
    mepa_bool_t                 enable;               /* Event Enable or Disable */
    uint32_t                    value_list[MAX_VALUE_LIST];
    uint32_t                    value_cnt;
} macsec_configuration;

