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
#include <mepa_apps/phy_macsec_demo.h>
#include "phy_demo_apps.h"


/**
 * \brief   : Enables MACsec Block for given ports
 * \command : "macsec enable"
 */
static void cli_cmd_macsec_ena(cli_req_t *req);

/**
 * \brief   : Disables MACsec Block for given ports
 * \command : "macsec disable"
 */
static void cli_cmd_macsec_dis(cli_req_t *req);

/**
 * \brief   : Status of MACsec Block on all ports
 * \command : "macsec port_state"
 */
static void cli_cmd_macsec_port_state(cli_req_t *req);

/**
 * \brief   : Create Secure Entity or Update the Configuration of SecY
 * \command : "secy_create_upd"
 */
static void cli_cmd_secy_create(cli_req_t *req);

/**
 * \brief   : Delete Secure Entity(SecY)
 * \command : "secy_del"
 */
static void cli_cmd_macsec_secy_del(cli_req_t *req);

/**
 * \brief   : Pattern matching parameters
 * \command : "match_conf"
 */
static void cli_cmd_match_param(cli_req_t *req);

/**
 * \brief   : Configure Pattern Match for SecY
 * \command : "pattern_conf"
 */
static void cli_cmd_match_conf(cli_req_t *req);
/**
 * \brief   : Start MACsec Application
 * \command : "macsec"
 */
static void cli_cmd_macsec_demo(cli_req_t *req);




meba_inst_t meba_macsec_instance;

static mscc_appl_trace_module_t trace_module = {
    .name = "macsec-demo"
};
static mscc_appl_trace_group_t trace_groups[10] = {
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

#define MAX_PORTS    20                   /* Number of Ports in EDSx */
#define TXT_DISABLED 0
#define TXT_ENABLED  1
#define TXT_BYPASSED 2
#define TXT_NOT_APP  3
#define TXT_NOT_TRIG 4
#define TXT_TRIG     5

#define MAX_STRING_LEN   200       /* Max length of string in File */
#define MAX_WORD_COUNT   1000      /* Max word count in File */
#define MAX_KEY_LEN      32        /* SAK key length */
#define KEY_LEN_128_BIT  16        /* Key Length for 128 bit Cipher */
#define KEY_LEN_256_BIT  32        /* Key Length for 256 bit Cipher */
#define MAX_HASK_KEY_LEN 16        /* SAK Hash key length */
#define MAX_SALT_KEY_LEN 12        /* SAK Salt length for XPN */
#define XPN_SSCI_LEN     4         /* Short Secure Channel Identifier length for XPN */
#define VSC_PHY_MAX_CP_RULES 26    /* Maximum Control Packet Rules supported for VSC PHYs */
#define LAN80XX_MAX_CP_RULES 18    /* Maximum Control Packet Rules Supported for LAN80XX PHYs */

#define MASK_16BIT       0xFFFF                 /* 16 Bit Mask value */
#define MASK_32BIT       0xFFFFFFFF             /* 32 Bit Mask value */
#define MASK_64BIT       0xFFFFFFFFFFFFFFFF     /* 64 Bit mask value */
#define NUMBER_ASCII_TO_DECIMAL 48
#define DIGIT_TO_NUMBER         10
FILE *fptr;
uint8_t key[MAX_KEY_LEN]         = {0};
uint8_t h_key[MAX_HASK_KEY_LEN]  = {0};
uint8_t salt[MAX_SALT_KEY_LEN]   = {0};
uint8_t ssci_xpn[XPN_SSCI_LEN]   = {0};
int macsec_poll_cnt = 0;

/* Structure objects to know the keyword parsed and to store the pattern values */
keyword_parsed keyword;
macsec_store   pattern_values;
macsec_event_status macsec_status[MAX_PORTS];

static int mepa_dev_check(meba_inst_t meba_instance, mepa_port_no_t port_no) {
    if(!meba_instance->phy_devices[port_no]) {
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}


const char *cli_capable_txt(mesa_bool_t enabled)
{
    return (enabled ? "Capable" : "NotCapable");
}

const char *cli_enable_txt(int status)
{
    switch(status) {
    case 0:
         return "Disabled";
         break;
    case 1:
         return "Enabled";
         break;
    case 2:
         return "Bypassed";
         break;
    case 3:
        return "  --";
        break;
    case 4:
        return "Not Triggered";
        break;
    case 5:
        return "Triggered";
        break;
    }
    return 0;
}

static void cli_cmd_macsec_ena(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_port_no_t  port_no;
    if(!req->set) {
        T_E("\nInvalid Argument");
        cli_printf("\n Syntax : %s\n", "macsec enable <port_list> [bypass_none|bypass_disable]");
        return;
    }
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if(!meba_macsec_instance->phy_devices[iport]) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
           return;
        }
        mepa_macsec_init_t init_get;
        mepa_rc rc;

        if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) != MEPA_RC_OK) {
            return;
        }
        if(mreq->bypass_conf == MEPA_MACSEC_INIT_BYPASS_DISABLE && init_get.enable != 1) {
            cli_printf("\n MACsec Block on port no %d is not in bypass to disable from bypass\n", iport);
           continue;
        }

        mepa_macsec_init_t init_data = { .enable = TRUE,
                                         .dis_ing_nm_macsec_en = TRUE,
                                         .mac_conf.lmac.dis_length_validate = FALSE,
                                         .mac_conf.hmac.dis_length_validate = FALSE,
                                         .bypass = mreq->bypass_conf};

        if ((rc = mepa_macsec_init_set(meba_macsec_instance->phy_devices[iport], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in Enabling MACsec on port : %d\n", iport);
            return;
        }
        /* Default action configuration for non-matching frames */
        mepa_macsec_default_action_policy_t default_action_policy = {
            .ingress_non_control_and_non_macsec = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_control_and_non_macsec     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_non_control_and_macsec     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_control_and_macsec         = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .egress_control                     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .egress_non_control                 = MEPA_MACSEC_DEFAULT_ACTION_DROP,
        };

        if ((rc = mepa_macsec_default_action_set(meba_macsec_instance->phy_devices[iport], iport, &default_action_policy)) != MEPA_RC_OK) {
            T_E("\n Error in configuration Default action set on port :%d \n", iport);
            return;
        }

        cli_printf("\n ...... MACsec Block Enabled on Port : %d......\n", iport);
    }
    return;
}

static void cli_cmd_macsec_dis(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc;
    mepa_macsec_init_t init_data;
    mepa_port_no_t  port_no;
    mepa_macsec_init_t init_get;

    if(!req->set) {
        T_E("\nInvalid Argument");
        cli_printf("\n Syntax : %s\n", "macsec disable <port_list> [bypass_none|bypass_enable]");
        return;
    }
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }

        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
            return;
        }

        if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) != MEPA_RC_OK) {
            return;
        }

        if(init_get.enable != TRUE && mreq->bypass_conf == MEPA_MACSEC_INIT_BYPASS_ENABLE) {
            cli_printf("\n Enable MAC Block for this port %d....\n", iport);
            return;
        }

        init_data.enable = FALSE;
        init_data.dis_ing_nm_macsec_en = TRUE;
        init_data.mac_conf.lmac.dis_length_validate = FALSE;
        init_data.mac_conf.hmac.dis_length_validate = FALSE;
        init_data.bypass = mreq->bypass_conf;

        if ((rc = mepa_macsec_init_set(meba_macsec_instance->phy_devices[iport], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in Enabling MACsec on port : %d\n", iport);
            return;
        }
        cli_printf("\n ...... MACsec Block Disabled on Port : %d......\n", iport);
    }
    return;
}

static void cli_cmd_macsec_port_state(cli_req_t *req)
{
    mepa_rc rc;
    mepa_bool_t macsec_capable;
    mepa_macsec_init_t init_get;
    mepa_bool_t capable = 0;
    int status = 0;
    printf("\n");
    cli_printf("Port    MACsec Capable    Status");
    cli_printf("\n-----------------------------------\n");
    printf("\n");

    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            capable = 0;
            status = TXT_NOT_APP;
            cli_printf("%-8u%-18s%s\n",
                   port_no,
                   cli_capable_txt(capable),
                   cli_enable_txt(status));
            continue;
        }

        if((rc = mepa_macsec_is_capable(meba_macsec_instance->phy_devices[iport], iport, &macsec_capable)) != MEPA_RC_NOT_IMPLEMENTED) {
            capable = macsec_capable;
            if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) == MEPA_RC_OK) {
                status = (init_get.enable == 1 && init_get.bypass != MEPA_MACSEC_INIT_BYPASS_ENABLE) ? TXT_ENABLED : TXT_DISABLED;
                if(init_get.bypass == MEPA_MACSEC_INIT_BYPASS_ENABLE)
                    status = TXT_BYPASSED;
            }
        } else {
            capable = 0;
            status = TXT_NOT_APP;
        }
        cli_printf("%-8u%-18s%s\n",
                   port_no,
                   cli_capable_txt(capable),
                   cli_enable_txt(status));
    }
    return;
}

static void cli_cmd_secy_create(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc = MEPA_RC_ERROR;
    int protect = 0, frame_val = 0, cipher_sel = 0, sectag_len = 0, es_bit = 0, scb_bit = 0;
    char input[3];
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_secy_conf_t secy_conf;
    if (mreq->port_id == 0xFFFF) {
        T_E("\n SecY ID not Supported on Port : %d \n", req->port_no);
        return;
    }

    cli_printf("\n SECTAG PARAMETERS CONFIGURATION ........................ \n");
    cli_printf("\n\n Protect Frames [1 : true, 0 : false] : ");
    scanf("%s", &input[0]);
    protect = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));
    if(protect == 0 || protect == 1) {
        secy_conf.protect_frames = protect;
    } else {
        T_E("\n Invalid input Press 0 or 1\n");
        return;
    }
    cli_printf("\n Select Frame Validation [0 : Disable, 1 : Strict , 2 : Check] : ");
    scanf("%s", &input[0]);
    frame_val = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));
    switch(frame_val) {
    case 0:
        secy_conf.validate_frames = MEPA_MACSEC_VALIDATE_FRAMES_DISABLED;
        break;
    case 1:
        secy_conf.validate_frames = MEPA_MACSEC_VALIDATE_FRAMES_STRICT;
        break;
    case 2:
        secy_conf.validate_frames = MEPA_MACSEC_VALIDATE_FRAMES_CHECK;
        break;
    default:
        T_E("\n Invalid Frame validation input\n");
        return;
    }
    cli_printf("\n Select Cipher Suit [0 : GCM-AES-128,  1 : GCM-AES-256,  2 : GCM-AES-XPN-128,  3 : GCM-AES-XPN-256] : ");
    scanf("%s", &input[0]);
    cipher_sel = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));
    switch(cipher_sel) {
    case 0:
        secy_conf.current_cipher_suite = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128;
        break;
    case 1:
        secy_conf.current_cipher_suite = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256;
        break;
    case 2:
        secy_conf.current_cipher_suite = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128;
        break;
    case 3:
        secy_conf.current_cipher_suite = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256;
        break;
    default:
        T_E("\n Invalid Cipher Suit Input \n");
        return;
    }
    cli_printf("\n Sectag length [0 : 16-byte Sectag,  1 : 8-Byte Sectag] : ");
    scanf("%s", &input[0]);
    sectag_len = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));    
    if(sectag_len == 0 || sectag_len == 1) {
        secy_conf.always_include_sci = !(sectag_len);
    } else {
        T_E("\n Invalid Sectag Length Input \n");
        return;
    }
    cli_printf("\n End Station(ES) Bit in Sectag (0 or 1) : ");
    scanf("%s", &input[0]);
    es_bit = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));
    if(es_bit == 0 || es_bit == 1) {
        secy_conf.use_es = es_bit;
    } else {
        T_E("\n Invalid End Station Bit Input\n");
        return;
    }
    cli_printf("\n Single Copy Broadcast (SCB) in Sectag (0 or 1) : ");
    scanf("%s", &input[0]);
    scb_bit = atoi_Conversion(input);
    memset(input, '\0', sizeof(input));
    if(scb_bit == 0 || scb_bit == 1) {
       secy_conf.use_scb = scb_bit;
    } else {
        T_E("\n invalid SCB Input\n");
	return;
    }
    cli_printf("\n");

    
    mepa_macsec_port_t     macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    secy_conf.replay_protect         = FALSE;
    secy_conf.replay_window          = 0;
    secy_conf.confidentiality_offset = 0;
    memcpy(&secy_conf.mac_addr, &mreq->mac_addr, sizeof(mepa_mac_t));

    if(mreq->secy_create) {
        if ((rc = mepa_macsec_secy_conf_add(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf)) != MEPA_RC_OK) {
            T_E("\n Error in creating the SecY on port : %d \n", req->port_no);
            return;
        }

        /* SecY Control port enable */
        if((rc = mepa_macsec_secy_controlled_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, TRUE)) != MEPA_RC_OK) {
            T_E("\n Error in enabling the SecY controlled port on port : %d \n", req->port_no);
            return;
        }
    } else {
        if ((rc = mepa_macsec_secy_conf_update(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf)) != MEPA_RC_OK) {
            T_E("\n Error in creating the Updating SecY Parameters on port : %d \n", req->port_no);
            return;
        }
    }
    cli_printf("\n ...... SecY %s on Port : %d with port_id : %d......\n", mreq->secy_create ? "Created" : "Updated", req->port_no, mreq->port_id);
    return;
}

static void cli_cmd_macsec_secy_del(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc = MEPA_RC_ERROR;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_macsec_secy_conf_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
        T_E("\n Error in Deleting the SecY on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n ....... SecY Deleted on Port : %d with port_id : %d...... \n", req->port_no, mreq->port_id);
}

static void cli_cmd_match_param(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    memcpy(&pattern_values.pattern_ethtype, &mreq->pattern_ethtype, sizeof(mepa_etype_t));
    memcpy(&pattern_values.pattern_src_mac_addr, &mreq->pattern_src_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_values.pattern_dst_mac_addr, &mreq->pattern_dst_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_values.vid, &mreq->vid, sizeof(mepa_vid_t));
    memcpy(&pattern_values.vid_inner, &mreq->vid_inner, sizeof(mepa_vid_t));
    cli_printf("\n ...... Match Parameters configured......\n");
    return;
}

static void cli_cmd_match_conf(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc;
    int match = 0;
    char input[3];
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : pattern_conf <port_no> port-id <port_id> [egress|ingress] [cont_port|uncont_port|drop]\n");
        return;
    } 
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t     macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    mepa_macsec_direction_t direction;
    /* Pattern Matching rule */
    mepa_macsec_match_pattern_t pattern_match;
    memset(&pattern_match, 0 , sizeof(mepa_macsec_match_pattern_t));

    cli_printf("\n 0 = Disable Pattern Matching");
    cli_printf("\n 1 = DMAC Pattern Matching");
    cli_printf("\n 2 = Ethertype Patter Matching");
    cli_printf("\n 3 = SMAC Pattern MAtching");
    cli_printf("\n 4 = DMAC and Ethertype Match");
    cli_printf("\n 5 = SMAC and Ethertype Match");
    cli_printf("\n 6 = DMAC and SMAC Matching ");
    cli_printf("\n 7 = DMAC, Ethertype and SMAC Matching");
    cli_printf("\n 8 = Outer VLAN ID Match");
    cli_printf("\n 9 = Inner VLAN ID Match");
    cli_printf("\n\n Parameter that needs to be Matched : ");
    scanf("%s", &input[0]);
    match = atoi_Conversion(input);
    cli_printf("\n\n");
    switch(match) {
    case 0:
        pattern_match.match = MEPA_MACSEC_MATCH_DISABLE;
        break;
    case 1:
        pattern_match.match = MEPA_MACSEC_MATCH_DMAC;
        break;
    case 2:
        pattern_match.match = MEPA_MACSEC_MATCH_ETYPE;
        break;
    case 3:
        pattern_match.match = MEPA_MACSEC_MATCH_SMAC;
        break;
    case 4:
        pattern_match.match = MEPA_MACSEC_MATCH_DMAC | MEPA_MACSEC_MATCH_ETYPE;
        break;
    case 5:
        pattern_match.match = MEPA_MACSEC_MATCH_SMAC | MEPA_MACSEC_MATCH_ETYPE;
        break;
    case 6:
        pattern_match.match = MEPA_MACSEC_MATCH_DMAC | MEPA_MACSEC_MATCH_SMAC;
        break;
    case 7:
        pattern_match.match = MEPA_MACSEC_MATCH_DMAC | MEPA_MACSEC_MATCH_SMAC | MEPA_MACSEC_MATCH_ETYPE;
        break;
    case 8:
        pattern_match.match = MEPA_MACSEC_MATCH_VLAN_ID | MEPA_MACSEC_MATCH_HAS_VLAN;
        break;
    case 9:
        pattern_match.match = MEPA_MACSEC_MATCH_VLAN_ID_INNER | MEPA_MACSEC_MATCH_HAS_VLAN_INNER;
        break;
    default:
        T_E("\n Invalid Pattern matching Parameter\n");
        return;
    }
    pattern_match.priority            = MEPA_MACSEC_MATCH_PRIORITY_HIGH;
    pattern_match.is_control          = TRUE;
    pattern_match.etype               = pattern_values.pattern_ethtype;
    pattern_match.vid                 = pattern_values.vid;
    pattern_match.vid_inner           = pattern_values.vid_inner;

    if(pattern_match.match & MEPA_MACSEC_MATCH_HAS_VLAN) {
        pattern_match.has_vlan_tag       = TRUE;
    }
    if(pattern_match.match & MEPA_MACSEC_MATCH_HAS_VLAN_INNER) {
        pattern_match.has_vlan_inner_tag = TRUE;
    }
    
    memcpy(&pattern_match.src_mac, &pattern_values.pattern_src_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_match.dest_mac, &pattern_values.pattern_dst_mac_addr, sizeof(mepa_mac_t));

    if(mreq->egress_direction) {
        direction =  MEPA_MACSEC_DIRECTION_EGRESS;
    } else {
        direction = MEPA_MACSEC_DIRECTION_INGRESS;
    }
    if ((rc = mepa_macsec_pattern_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, direction, mreq->match_action, &pattern_match)) != MEPA_RC_OK) {
        T_E("\n Error in Configuring Pattern matching rule on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n ...... Pattern Matching rule is Configured ......\n");
    return;
}

static void cli_cmd_tx_sc_create(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    /* Tx Secure channel Creation */
    if((rc = mepa_macsec_tx_sc_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
        T_E("\n Error in creating Transmit Secure channel on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n...... Transmit Secure Channel Created on port %d for SecY with is :%d......\n",req->port_no, mreq->port_id);
    return;
}

static void cli_cmd_macsec_tx_sc_del(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    /* Tx Secure channel Creation */
    if((rc = mepa_macsec_tx_sc_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
        T_E("\n Error in deleting Transmit Secure channel on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Transmit Secure Channel Deleted on port %d for SecY with Port id :%d......\n",req->port_no, mreq->port_id);
    return;
}

static void cli_cmd_rx_sc_create(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_secy_conf_t secy_conf_get;
    mepa_rc rc;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    /* Rx Secure channel Creation */
    if((rc = mepa_macsec_rx_sc_add(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci)) != MEPA_RC_OK) {
        T_E("\n Error in Rx Secure Channel Create on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Receive Secure Channel Created on port %d for SecY with Port id :%d......\n",req->port_no, mreq->port_id);

    return;
}

static void cli_cmd_macsec_rx_sc_del(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_secy_conf_t secy_conf_get;
    mepa_rc rc;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    /* Rx Secure channel Delete */
    if((rc = mepa_macsec_rx_sc_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci)) != MEPA_RC_OK) {
        T_E("\n Error in Rx Secure Channel Deleted on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Receive Secure Channel Deleted on port %d for SecY with Port id :%d......\n",req->port_no, mreq->port_id);
    return;
}

static void file_parse_to_get_key(char *filestring)
{
    char myString[MAX_STRING_LEN];
    char *stx_words[MAX_WORD_COUNT];
    int found_buf = 0, found_hashbuf = 0, found_salt = 0, stx_count = 0, found_ssci = 0;
    uint8_t value[MAX_KEY_LEN] = {0};
    cli_build_words(filestring, &stx_count, stx_words,1);
    for(int k = 0; k < stx_count; k++) {
	memset(value, 0, sizeof(value));
        if(strcmp(stx_words[k], "'buf':") == 0) {
            found_buf = 1;
            continue;
        }
        if(strcmp(stx_words[k], "'h_buf':") == 0) {
            found_hashbuf = 1;
            continue;
        }
        if(strcmp(stx_words[k], "'s_buf':") == 0) {
            found_salt = 1;
            continue;
        }
        if(strcmp(stx_words[k], "'ssci':") == 0) {
            found_ssci = 1;
            continue;
        }
        if(found_buf || found_hashbuf || found_salt || found_ssci) {
            int j = 0;
            memcpy(myString, stx_words[k], sizeof(myString));
            for(int i = 0; myString[i] != ']'; i++) {
                if(myString[i] == '[')
                    continue;
                if (myString[i] == ',') {
                    j++;
               }
               else {
                   value[j] = value[j] * DIGIT_TO_NUMBER + (myString[i] - NUMBER_ASCII_TO_DECIMAL);
               }
            }
	    if(found_buf) {
                memcpy(key, value, sizeof(key));
                found_buf = 0;
            } if(found_hashbuf) {
                memcpy(h_key, value, sizeof(h_key));
                found_hashbuf = 0;
            } if(found_salt) {
                memcpy(salt, value, sizeof(salt));
                found_salt = 0;
            } if(found_ssci) {
                memcpy(ssci_xpn, value, sizeof(ssci_xpn));
                found_ssci = 0;
            }
        }
    }
}

static void cli_cmd_tx_sa_create(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    char filename[100];
    mepa_macsec_secy_conf_t secy_conf_get;
    mepa_macsec_ssci_t ssci;

    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf(" Syntax : tx_sa_create <port_no> port-id <port_id> an <an_no> next-pn <pn> conf [true|false] \n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    uint32_t next_pn_nxpn;
    char myString[MAX_STRING_LEN];
    char filestring[MAX_WORD_COUNT];
    char file_location[] = "/root/";
    cli_printf("\n");
    cli_printf("\td - Default File for MACsec SAK \"macsec_key.json\"");
    cli_printf("\n\tEnter MACsec SAK File Name default(press d) or enter file name : ");
    memset(filename, 0, sizeof(filename));
    scanf("%s", filename);

    if ((strlen(filename) == 1) && (filename[0] == 'd' || filename[0] == 'D')) {
        strcat(file_location, "mepa_scripts/macsec_key.json");
    } else {
        if(strcmp(&filename[strlen(filename) - 5], ".json") != 0) {
            T_E("\n File should be in .json formate \n");
            return;
        }
        strcat(file_location, filename);
    }
    memset(filestring, 0, sizeof(filestring));
    if ((fptr = fopen(file_location, "r")) != NULL) {
        while(fgets(myString, sizeof(myString), fptr)) {
            strcat(filestring, myString);
        }
        fclose(fptr);
    } else{
        T_E("\n Error in opening the %s file .......\n", file_location);
        return;
    }        
    file_parse_to_get_key(filestring);

    mepa_macsec_pkt_num_t next_pkt;

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    mepa_macsec_sak_t sak;
    memcpy(sak.buf, key, sizeof(key));
    memcpy(sak.h_buf, h_key, sizeof(h_key));
    memcpy(sak.salt.buf, salt, sizeof(salt));
    memcpy(ssci.buf, ssci_xpn, sizeof(ssci_xpn));

    /*Assigning Global Variables to Zero */
    memset(key, 0, sizeof(key));
    memset(h_key, 0, sizeof(h_key));
    memset(salt, 0, sizeof(salt));
    memset(ssci_xpn, 0, sizeof(ssci_xpn));
    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY Config on port : %d \n", req->port_no);
        return;
    }

    if(secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128 || secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128) {
        sak.len = KEY_LEN_128_BIT;
    } else {
       sak.len = KEY_LEN_256_BIT;
    }

    if(secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256 ||secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128){
        next_pkt.xpn = mreq->next_pn;
        if((rc = mepa_macsec_tx_seca_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no, next_pkt, mreq->conf, &sak, &ssci)) != MEPA_RC_OK)
        {
            T_E("Error is creating Tx Secure asoosiation on port %d\n", req->port_no);
            return;
        }
    } else {
        if(mreq->next_pn > (MASK_32BIT - 1)) {
             T_E("Selcted Cipher Suit is NON-XPN the PN should be less than 2^32\n");
        }
        next_pn_nxpn = mreq->next_pn & MASK_32BIT;
        if((rc = mepa_macsec_tx_sa_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no, next_pn_nxpn, mreq->conf, &sak)) != MEPA_RC_OK) {
            T_E("Error is creating Tx Secure asoosiation on port %d\n", req->port_no);
            return;
        }
    }

    if((rc = mepa_macsec_tx_sa_activate(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no)) != MEPA_RC_OK) {
        T_E("\nError in Activating Tx Secure Assosiation on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Transmit Secure Association an = %d Created for Channel id :%d ......\n",mreq->an_no, mreq->port_id);
    return;
}

static void cli_cmd_rx_sa_create(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    char filename[100];
    mepa_macsec_pkt_num_t next_pkt;
    mepa_macsec_ssci_t ssci;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    uint32_t lowest_pn_nxpn;
    char myString[MAX_STRING_LEN];
    char filestring[MAX_WORD_COUNT];
    char file_location[] = "/root/";
    cli_printf("\n");
    cli_printf("\td - Default File for MACsec SAK \"macsec_key.json\"");
    cli_printf("\n\tEnter MACsec SAK File Name default(press d) or enter file name : ");
    memset(filename, 0, sizeof(filename));
    scanf("%s", filename);

    if ((strlen(filename) == 1) && (filename[0] == 'd' || filename[0] == 'D')) {
        strcat(file_location, "mepa_scripts/macsec_key.json");
    } else {
        if(strcmp(&filename[strlen(filename) - 5], ".json") != 0) {
            T_E("\n File should be in .json formate \n");
            return;
        }
        strcat(file_location, filename);
    }
    memset(filestring, 0, sizeof(filestring));
    if ((fptr = fopen(file_location, "r")) != NULL) {
        while(fgets(myString, sizeof(myString), fptr)) {
            strcat(filestring, myString);
        }
        fclose(fptr);
    } else {
        T_E("\n Error in opening %s file .......\n", file_location);
        return;
    }
    file_parse_to_get_key(filestring);

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    mepa_macsec_secy_conf_t secy_conf_get;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    mepa_macsec_sak_t sak;
    memcpy(sak.buf, key, sizeof(key));
    memcpy(sak.h_buf, h_key, sizeof(h_key));
    memcpy(sak.salt.buf, salt, sizeof(salt));
    memcpy(ssci.buf, ssci_xpn, sizeof(ssci_xpn));

    /*Assigning Global Variables to Zero */
    memset(key, 0, sizeof(key));
    memset(h_key, 0, sizeof(h_key));
    memset(salt, 0, sizeof(salt));
    memset(ssci_xpn, 0, sizeof(ssci_xpn));

    if(secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128 || secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128) {
        sak.len = KEY_LEN_128_BIT;
    } else {
       sak.len = KEY_LEN_256_BIT;
    }

    if(secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256 ||secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128) {
        next_pkt.xpn = mreq->lowest_pn;
        if((rc = mepa_macsec_rx_seca_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no, next_pkt, &sak, &ssci)) != MEPA_RC_OK) {
            T_E("Error is creating Rx Secure asoosiation on port %d\n", req->port_no);
            return;
        }
    } else {
        if(mreq->lowest_pn > (MASK_32BIT - 1)) {
             T_E("Selcted Cipher Suit is NON-XPN the PN should be less than 2^32\n");
        }
        lowest_pn_nxpn = mreq->lowest_pn & MASK_32BIT;

        if((rc = mepa_macsec_rx_sa_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no, lowest_pn_nxpn, &sak)) != MEPA_RC_OK) {
            T_E("Error is creating Rx Secure asoosiation on port %d\n", req->port_no);
            return;
        }
    }
    if((rc = mepa_macsec_rx_sa_activate(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no)) != MEPA_RC_OK) {
        T_E("\n Error in activating Rx Secure Assosiation on port : %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Receive Secure Association an = %d Created for Channel id :%d ......\n",mreq->an_no, mreq->rx_sc_id);
    return;
}


static void cli_cmd_macsec_tx_sa_del(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if((rc = mepa_macsec_tx_sa_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no)) != MEPA_RC_OK) {
        T_E("Error is Deleting Tx Secure asoosiation on port %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Transmit Secure Association an = %d Deleted for Channel id :%d ......\n",mreq->an_no, mreq->port_id);
    return;
}

static void cli_cmd_macsec_rx_sa_del(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    mepa_macsec_secy_conf_t secy_conf_get;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    if((rc = mepa_macsec_rx_sa_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no)) != MEPA_RC_OK) {
        T_E("Error is Deleting Rx Secure asoosiation on port %d\n", req->port_no);
        return;
    }
    cli_printf("\n ...... Receive Secure Association an = %d Deleted for Channel id :%d ......\n",mreq->an_no, mreq->rx_sc_id);
    return;
}

static void cli_cmd_rx_sa_pn_update(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_pkt_num_t next_pkt;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    uint32_t lowest_pn_nxpn;
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    mepa_macsec_secy_conf_t secy_conf_get;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    if(secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256 ||secy_conf_get.current_cipher_suite== MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128){
        next_pkt.xpn = mreq->lowest_pn;
        if((rc = mepa_macsec_rx_seca_lowest_pn_update(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no, next_pkt)) != MEPA_RC_OK) {
            T_E("Error is Updating Next PN on port %d\n", req->port_no);
            return;
        }
    } else {

        if(mreq->lowest_pn > MASK_32BIT) {
             T_E("Selcted Cipher Suit is NON-XPN the PN should be less than 2^32\n");
        }
        lowest_pn_nxpn = mreq->lowest_pn & MASK_32BIT;

        if((rc = mepa_macsec_rx_sa_lowest_pn_update(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no, lowest_pn_nxpn)) != MEPA_RC_OK) {
            T_E("Error is Updating Next PN on port %d\n", req->port_no);
            return;
        }
    }
    cli_printf("\n ...... Receive Secure Association an = %d PN Updated for Channel id :%d with PN = %d......\n",mreq->an_no, mreq->rx_sc_id, mreq->lowest_pn);
    return;
}

static void cli_macsec_statistics(const char *col1, const char *col2, uint64_t value1, uint64_t value2)
{
    if(strcmp(col2, "NULL") == 0) {
        cli_printf("%-30s %s %-15d\n", col1, ":", value1);
        return;
    }
    cli_printf("%-30s %s %-15ld %-30s %s %-15ld\n", col1, ":", value1, col2, ":", value2);     
}                     

static void cli_cmd_macsec_secy_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;

    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics secy <port_no> port-id <port_id> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    if(!mreq->statistics_get) {
        if ((rc = mepa_macsec_secy_counters_clear(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
            T_E("\n Error in Clearing the SecY Statistics on port : %d \n", req->port_no);
            return;
        }
	cli_printf("\n SecY Statistics Cleared on Port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_secy_counters_t counters;
    if ((rc = mepa_macsec_secy_counters_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &counters)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY Statistics on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n\n");
    cli_printf("Secy Statistics\n");
    cli_printf("============================\n");
    cli_macsec_statistics("in_pkts_untagged", "in_pkts_no_tag", counters.in_pkts_untagged, counters.in_pkts_no_tag);
    cli_macsec_statistics("in_pkts_bad_tag", "in_pkts_unknown_sci", counters.in_pkts_bad_tag, counters.in_pkts_unknown_sci);
    cli_macsec_statistics("in_pkts_no_sci", "in_pkts_overrun", counters.in_pkts_no_sci, counters.in_pkts_overrun);
    cli_macsec_statistics("in_octets_validated", "in_octets_decrypted", counters.in_octets_validated, counters.in_octets_decrypted);
    cli_macsec_statistics("out_pkts_untagged", "out_pkts_too_long", counters.out_pkts_untagged, counters.out_pkts_too_long);
    cli_macsec_statistics("out_octets_protected", "out_octets_encrypted", counters.out_octets_protected, counters.out_octets_encrypted);
    return;
}

static void cli_cmd_macsec_tx_sc_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics tx_sc <port_no> port-id <port_id> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    if(!mreq->statistics_get) {
        if ((rc = mepa_macsec_txsc_counters_clear(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
            T_E("\n Error in Clearing the Transmit Secure Channel Statistics on port : %d \n", req->port_no);
            return;
        }
	cli_printf("\n TX SC Statistics Cleared on Port : %d \n", req->port_no);
        return;
    }
    mepa_macsec_tx_sc_counters_t counters;
    if ((rc = mepa_macsec_tx_sc_counters_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &counters)) != MEPA_RC_OK) {
        T_E("\n Error in getting the Transmit Secure Channel Statistics on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n\n");
    cli_printf("Transmit Secure Channel Statistics\n");
    cli_printf("===============================================\n");
    cli_macsec_statistics("out_pkts_protected", "out_pkts_encrypted", counters.out_pkts_protected, counters.out_pkts_encrypted);
    cli_macsec_statistics("out_octets_protected", "out_octets_encrypted", counters.out_octets_protected, counters.out_octets_encrypted);
    return;
}

static void cli_cmd_macsec_tx_sa_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics tx_sa <port_no> port-id <port_id> an <an_no> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    if(!mreq->statistics_get) {
        if ((rc = mepa_macsec_txsa_counters_clear(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no)) != MEPA_RC_OK) {
            T_E("\n Error in Clearing the Transmit Secure Association Statistics on port : %d \n", req->port_no);
            return;
        }
	cli_printf("\n TX SA Statistics Cleared on Port : %d with AN %d\n", req->port_no, mreq->an_no);
        return;
    }
    mepa_macsec_tx_sa_counters_t counters;
    if ((rc = mepa_macsec_tx_sa_counters_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no, &counters)) != MEPA_RC_OK) {
        T_E("\n Error in getting the Transmit Secure Association Statistics on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n\n");
    cli_printf("Transmit Secure Association Statistics \n");
    cli_printf("==================================================\n");
    cli_printf("%-30s %s %-15ld\n", "out_pkts_protected", ":", counters.out_pkts_protected);
    cli_printf("%-30s %s %-15ld\n", "out_pkts_encrypted", ":", counters.out_pkts_encrypted);
    return;
}

static void cli_cmd_macsec_rx_sc_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics rx_sc <port_no> port-id <port_id> sc-id <id> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    mepa_macsec_secy_conf_t secy_conf_get;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    if(!mreq->statistics_get) {
        if ((rc = mepa_macsec_rxsc_counters_clear(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci)) != MEPA_RC_OK) {
            T_E("\n Error in Clearing the Receive Secure Channel Statistics on port : %d \n", req->port_no);
            return;
        }
        cli_printf("\n RX SC Statistics Cleared on Port : %d \n", req->port_no);	
        return;
    }
    mepa_macsec_rx_sc_counters_t counters;
    if ((rc = mepa_macsec_rx_sc_counters_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, &counters)) != MEPA_RC_OK) {
        T_E("\n Error in getting the Receive Secure Channel Statistics on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n\n");
    cli_printf("Receive Secure Channel Statistics \n");
    cli_printf("=================================================\n");
    cli_macsec_statistics("in_pkts_unchecked", "in_pkts_delayed", counters.in_pkts_unchecked, counters.in_pkts_delayed);
    cli_macsec_statistics("in_pkts_late", "in_pkts_ok", counters.in_pkts_late, counters.in_pkts_ok);
    cli_macsec_statistics("in_pkts_invalid", "in_pkts_not_valid", counters.in_pkts_invalid, counters.in_pkts_not_valid);
    cli_macsec_statistics("in_pkts_not_using_sa", "in_pkts_unused_sa", counters.in_pkts_not_using_sa, counters.in_pkts_unused_sa);
    cli_macsec_statistics("in_octets_validated", "in_octets_decrypted", counters.in_octets_validated, counters.in_octets_decrypted);
    return;
}

static void cli_cmd_macsec_rx_sa_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics rx_sa <port_no> port-id <port_id> sc-id <id> an <an_no> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;
    mepa_macsec_secy_conf_t secy_conf_get;

    if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
        T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
        return;
    }

    mepa_macsec_sci_t sci;
    memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
    sci.port_id = mreq->rx_sc_id;

    if(!mreq->statistics_get) {
        if ((rc = mepa_macsec_rxsa_counters_clear(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no)) != MEPA_RC_OK) {
            T_E("\n Error in Clearing the Receive Secure Association Statistics on port : %d \n", req->port_no);
            return;
        }
	cli_printf("\n RX SA Statistics Cleared on Port : %d with AN %d\n", req->port_no, mreq->an_no);
        return;
    }
    mepa_macsec_rx_sa_counters_t counters;
    if ((rc = mepa_macsec_rx_sa_counters_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no, &counters)) != MEPA_RC_OK) {
        T_E("\n Error in getting the Receive Secure Association Statistics on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n\n");
    cli_printf("Receive Secure Association Statistics \n");
    cli_printf("===================================================\n");
    cli_macsec_statistics("in_pkts_ok", "in_pkts_invalid", counters.in_pkts_ok, counters.in_pkts_invalid);
    cli_macsec_statistics("in_pkts_not_valid", "in_pkts_not_using_sa", counters.in_pkts_not_valid, counters.in_pkts_not_using_sa);
    cli_macsec_statistics("in_pkts_unchecked", "in_pkts_delayed", counters.in_pkts_unchecked, counters.in_pkts_delayed);
    cli_macsec_statistics("in_pkts_late", "NULL", counters.in_pkts_late, 0);

    return;
}

static void cli_cmd_macsec_mac_statistics(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n Invalid Syntax\n");
        cli_printf("\n Syntax : statistics mac [hmac|lmac] <port_no> [get|clear]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if(!mreq->statistics_get) {
        if(mreq->hmac_counters) {
            if ((rc = mepa_macsec_hmac_counters_clear(meba_macsec_instance->phy_devices[req->port_no], req->port_no)) != MEPA_RC_OK) {
                T_E("\n Error in Clearing HMAC Counters on port : %d \n", req->port_no);
                return;
             }
        } else{
            if ((rc = mepa_macsec_lmac_counters_clear(meba_macsec_instance->phy_devices[req->port_no], req->port_no)) != MEPA_RC_OK) {
                T_E("\n Error in Clearing LMAC Counters on port : %d \n", req->port_no);
                return;
            }
        }
	cli_printf("\n %s Counters cleared on Port : %d\n", mreq->hmac_counters ? "HMAC" : "LMAC", req->port_no);
        return;
    }
    mepa_macsec_mac_counters_t counters;
    if(mreq->hmac_counters) {
         if ((rc = mepa_macsec_hmac_counters_get(meba_macsec_instance->phy_devices[req->port_no], req->port_no, &counters, FALSE)) != MEPA_RC_OK) {
             T_E("\n Error in Getting HMAC Counters on port : %d \n", req->port_no);
             return;
         }
         cli_printf("\n\n");
         cli_printf("HOST MAC Statistics \n");
         cli_printf("============================\n");
    } else{
        if ((rc = mepa_macsec_lmac_counters_get(meba_macsec_instance->phy_devices[req->port_no], req->port_no, &counters, FALSE)) != MEPA_RC_OK) {
            T_E("\n Error in Getting LMAC Counters on port : %d \n", req->port_no);
            return;
        }
        cli_printf("\n\n");
        cli_printf("LINE MAC Statistics \n");
        cli_printf("============================\n");
    }
    cli_macsec_statistics("if_rx_octets", "if_rx_in_bytes", counters.if_rx_octets, counters.if_rx_in_bytes);
    cli_macsec_statistics("if_rx_pause_pkts", "if_rx_ucast_pkts", counters.if_rx_pause_pkts, counters.if_rx_ucast_pkts);
    cli_macsec_statistics("if_rx_multicast_pkts", "if_rx_broadcast_pkts", counters.if_rx_multicast_pkts, counters.if_rx_broadcast_pkts);
    cli_macsec_statistics("if_rx_discards", "if_rx_errors", counters.if_rx_discards, counters.if_rx_errors);
    cli_macsec_statistics("if_rx_StatsPkts", "if_rx_CRCAlignErrors", counters.if_rx_StatsPkts, counters.if_rx_CRCAlignErrors);
    cli_macsec_statistics("if_rx_UndersizePkts", "if_rx_OversizePkts", counters.if_rx_UndersizePkts, counters.if_rx_OversizePkts);
    cli_macsec_statistics("if_rx_Fragments", "if_rx_Jabbers", counters.if_rx_Fragments, counters.if_rx_Jabbers);
    cli_macsec_statistics("if_rx_Pkts64Octets", "if_rx_Pkts65to127Octets", counters.if_rx_Pkts64Octets, counters.if_rx_Pkts65to127Octets);
    cli_macsec_statistics("if_rx_Pkts128to255Octets", "if_rx_Pkts256to511Octets", counters.if_rx_Pkts128to255Octets, counters.if_rx_Pkts256to511Octets);
    cli_macsec_statistics("if_rx_Pkts512to1023Octets", "if_rx_Pkts1024to1518Octets", counters.if_rx_Pkts512to1023Octets, counters.if_rx_Pkts1024to1518Octets);
    cli_macsec_statistics("if_rx_Pkts1519toMaxOctets", "NULL", counters.if_rx_Pkts1519toMaxOctets, 0);
    cli_printf("\n\n");
    cli_macsec_statistics("if_tx_octets", "if_tx_errors", counters.if_tx_octets, counters.if_tx_errors);
    cli_macsec_statistics("if_tx_pause_pkts", "if_tx_ucast_pkts", counters.if_tx_pause_pkts, counters.if_tx_ucast_pkts);
    cli_macsec_statistics("if_tx_multicast_pkts", "if_tx_broadcast_pkts", counters.if_tx_multicast_pkts, counters.if_tx_broadcast_pkts);
    cli_macsec_statistics("if_tx_StatsPkts", "if_tx_DropEvents", counters.if_tx_StatsPkts, counters.if_tx_DropEvents);
    cli_macsec_statistics("if_tx_Pkts64Octets", "if_tx_Pkts65to127Octets", counters.if_tx_Pkts64Octets, counters.if_tx_Pkts65to127Octets);
    cli_macsec_statistics("if_tx_Pkts128to255Octets", "if_tx_Pkts256to511Octets", counters.if_tx_Pkts128to255Octets, counters.if_tx_Pkts256to511Octets);
    cli_macsec_statistics("if_tx_Pkts512to1023Octets", "if_tx_Pkts1024to1518Octets", counters.if_tx_Pkts512to1023Octets, counters.if_tx_Pkts1024to1518Octets);
    cli_macsec_statistics("if_tx_Pkts1519toMaxOctets", "if_tx_Collisions", counters.if_tx_Pkts1519toMaxOctets, counters.if_tx_Collisions);
    return;
}

static void cli_cmd_macsec_vlan_bypass(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n invalid Syntax\n");
        cli_printf("Syntax : vlan_bypass <port_no> port-id <port_id> [zero|one|two|three|four]\n");
        return;
    }
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_bypass_mode_t bypass_mode = {
        .mode             = MEPA_MACSEC_BYPASS_TAG,
        .hdr_bypass_len   = 0,
        .hdr_etype        = 0,
    };
    if ((rc = mepa_macsec_bypass_mode_set(meba_macsec_instance->phy_devices[req->port_no], req->port_no, &bypass_mode)) != MEPA_RC_OK) {
         T_E("\n Error in Configuring VLAN Bypass Configuring : %d \n", req->port_no);
         return;
    }

    mepa_macsec_tag_bypass_t tag;
    switch(mreq->vlan_tag) {
    case 0:
        tag = MEPA_MACSEC_BYPASS_TAG_ZERO;
        break;
    case 1:
        tag = MEPA_MACSEC_BYPASS_TAG_ONE;
        break;
    case 2:
        tag = MEPA_MACSEC_BYPASS_TAG_TWO;
        break;
    case 3:
        tag = MEPA_MACSEC_BYPASS_TAG_THREE;
        break;
    case 4:
        tag = MEPA_MACSEC_BYPASS_TAG_FOUR;
        break;
    default:
        tag = MEPA_MACSEC_BYPASS_TAG_ZERO;
        break;
    }
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_macsec_bypass_tag_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, tag)) != MEPA_RC_OK) {
         T_E("\n Error in Configuring VLAN Bypass Configuring : %d \n", req->port_no);
         return;
    }
    cli_printf("\n ...... VLAN Bypass Configured for %d tag on port no %d with SecY id %d......\n", mreq->vlan_tag , req->port_no, mreq->port_id);
    return;
}

static void cli_cmd_macsec_frame_capt(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if(!req->set) {
        T_E("\n invalid Syntax\n");
        cli_printf("Syntax : frame capture <port_list> [egress|ingress]\n");
        return;
    }
    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
            return;
        }
        mepa_macsec_frame_capture_t frame_capt;
        if(mreq->egress_direction) {
            frame_capt = MEPA_MACSEC_FRAME_CAPTURE_EGRESS;
        } else {
            frame_capt = MEPA_MACSEC_FRAME_CAPTURE_INGRESS;
        }
        if ((rc = mepa_macsec_frame_capture_set(meba_macsec_instance->phy_devices[iport], iport, frame_capt)) != MEPA_RC_OK) {
            T_E("\n Error in Configuring MACsec FIFO to Capture : %d \n", iport);
            return;
        }
        cli_printf("\n ...... MACsec FIFO Enabled on port %d send packets to capture ......\n", iport);
    }
    return;
}

static void cli_cmd_macsec_frame_get(cli_req_t *req)
{
    mepa_rc rc;
    uint32_t frame_length = 0;
    uint8_t  frame[MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX];
    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        memset(&frame, 0 ,sizeof(frame));
        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
            return;
        }
        if ((rc = mepa_macsec_frame_get(meba_macsec_instance->phy_devices[iport], iport, MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX, &frame_length, &frame[0])) != MEPA_RC_OK) {
            T_E("\n Error in Reading the Frame from MACsec FIFO on port : %d \n", iport);
            return;
        }
        cli_printf("\n Frame Captured on Port %d", iport);
        cli_printf("\n===============================================\n\n");
        cli_printf("\n Length of Frame Captured : %d\n", frame_length);
        for(int j = 0; j < frame_length; j++) {
            if(j%16 == 0) {
                cli_printf("\n0x");
                cli_printf("%-3x|", j);
            }
            if(j%8 == 0) {
                cli_printf(" ");
            }
            cli_printf("%02x ", frame[j]);
        }
        cli_printf("\n");
    }
    return;
}

static void macsec_poll(meba_inst_t inst)
{
    mepa_port_no_t port_no;
    mepa_macsec_event_t evt_mask;
    mepa_macsec_event_t evt_get;
    mepa_rc rc;
    for (port_no = 0; port_no < MAX_PORTS; port_no++) {
        if ((rc = mepa_dev_check(meba_macsec_instance, port_no)) != MEPA_RC_OK) {
             memset(&macsec_status[port_no], 0 ,sizeof(macsec_status[port_no]));
             continue;
        }
        mepa_macsec_init_t init_get;
        if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[port_no], &init_get)) != MEPA_RC_OK) {
            memset(&macsec_status[port_no], 0 ,sizeof(macsec_status[port_no]));
            continue;
        }
        if(init_get.enable != TRUE) {
            memset(&macsec_status[port_no], 0 ,sizeof(macsec_status[port_no]));
            continue;
        }
        if ((rc = mepa_macsec_event_enable_get(meba_macsec_instance->phy_devices[port_no], port_no, &evt_get)) != MEPA_RC_OK) {
            T_E("\n Error in Polling the MACsec Events on port : %d \n", port_no);
            return;
        }
        switch(evt_get) {
        case MEPA_MACSEC_SEQ_THRESHOLD_EVENT:
            macsec_status[port_no].seq_thres_event = 1;
            break;
        case MEPA_MACSEC_SEQ_ROLLOVER_EVENT:
            macsec_status[port_no].rollover_event = 1;
            break;
        case MEPA_MACSEC_SEQ_ALL:
            macsec_status[port_no].rollover_event = 1;
            macsec_status[port_no].seq_thres_event = 1;
            break;
        default:
            macsec_status[port_no].rollover_event = 0;
            macsec_status[port_no].seq_thres_event = 0;
            break;
        }
        if ((rc = mepa_macsec_event_poll(meba_macsec_instance->phy_devices[port_no], port_no, &evt_mask)) != MEPA_RC_OK) {
            T_E("\n Error in Polling the MACsec Events on port : %d \n", port_no);
            return;
        }
        switch(evt_mask) {
        case MEPA_MACSEC_SEQ_THRESHOLD_EVENT:
            macsec_status[port_no].seq_thres_status = 1;
            break;
        case MEPA_MACSEC_SEQ_ROLLOVER_EVENT:
            macsec_status[port_no].rollover_status = 1;
            break;
        case MEPA_MACSEC_SEQ_ALL:
            macsec_status[port_no].rollover_status = 1;
            macsec_status[port_no].seq_thres_status = 1;
            break;
        default:
            macsec_status[port_no].rollover_status = 0;
            macsec_status[port_no].seq_thres_status = 0;
            break;
        }
    }
    macsec_poll_cnt++;
    if(macsec_poll_cnt == 5) {
        for (port_no = 0; port_no < MAX_PORTS; port_no++) {
            if(macsec_status[port_no].rollover_status == 1) {
                cli_printf("Packet number rollover event Ocuured on port no : %d\n", port_no);
            }
            if(macsec_status[port_no].seq_thres_status == 1) {
                cli_printf("Sequence Number Threshold Packet Number Reached on port no : %d\n", port_no);
            }
        }
        macsec_poll_cnt = 0;
    }
    return;
}

static void cli_cmd_macsec_event_set(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    mepa_port_no_t  port_no;
    if(!req->set) {
        T_E("\n Invalid Syntax \n");
        cli_printf(" Syntax : macsec event set <port_list> [rollover|seq_threshold] [evt_enable|evt_disable]\n");
        return;
    }
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
            return;
        }
        mepa_macsec_event_t event;
        event = MEPA_MACSEC_SEQ_NONE;
        if(mreq->rollover_event) {
            event = MEPA_MACSEC_SEQ_ROLLOVER_EVENT;
        }
        if(mreq->sequence_threshold_event) {
            event = MEPA_MACSEC_SEQ_THRESHOLD_EVENT;
        }

        if ((rc = mepa_macsec_event_enable_set(meba_macsec_instance->phy_devices[iport], iport, event, mreq->enable)) != MEPA_RC_OK) {
            T_E("\n Error in Configuring MACsec Event on Port : %d \n", iport);
            return;
        }
    }
    return;
}

static void cli_cmd_macsec_event_get(cli_req_t *req)
{
    mepa_port_no_t port_no;
    cli_printf("\nPort No   Rollover event     Threshold event   Rollover Status    Threshold Status\n");
    cli_printf("--------------------------------------------------------------------------------------\n");
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        cli_printf("%-10d%-19s%-18s",port_no, cli_enable_txt(macsec_status[iport].rollover_event), cli_enable_txt(macsec_status[iport].seq_thres_event));
        if(macsec_status[iport].rollover_event == 0) {
            cli_printf(" %-17s", cli_enable_txt(3));
        } else {
            cli_printf("%-17s", cli_enable_txt(macsec_status[iport].rollover_status + 4));
        }
        if(macsec_status[iport].seq_thres_event == 0) {
            cli_printf(" %-17s", cli_enable_txt(3));
        } else {
            cli_printf("%-17s", cli_enable_txt(macsec_status[iport].seq_thres_status + 4));
        }
        cli_printf("\n");
    }
    cli_printf("\n\n");
    return;
}

static void cli_cmd_macsec_seq_num_set(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    uint32_t threshold;
    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if(mreq->statistics_get) {
            if ((rc = mepa_macsec_event_seq_threshold_get(meba_macsec_instance->phy_devices[iport], iport, &threshold)) != MEPA_RC_OK) {
                T_E("\n Error in getting the Seq number Threshold on Port : %d \n", iport);
                return;
            }
            cli_printf("\n Sequence Number threshold on port %ld is    : %d\n", iport, threshold);
        } else {
           if(mreq->seq_threshold_value == 0) {
               T_E("\n Sequence Threshold Value Cannot be zero configure value on port : %d \n", iport);
               return;
           }
           if ((rc = mepa_macsec_event_seq_threshold_set(meba_macsec_instance->phy_devices[iport], iport, mreq->seq_threshold_value)) != MEPA_RC_OK) {
               T_E("\n Error in Configuring the Seq number Threshold on Port : %d \n", iport);
               return;
           }
        }
    }
    return;
}

static void cli_cmd_macsec_conf_get(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    mepa_macsec_secy_conf_t secy_conf_get;
    mepa_macsec_tx_sc_conf_t tx_sc_conf_get;
    mepa_macsec_sak_t   sak;
    mepa_macsec_sci_t sci;
    mepa_macsec_rx_sc_conf_t  rx_sc_conf;
    mepa_macsec_pkt_num_t next_pn_xpn;
    memset(&secy_conf_get, 0, sizeof(mepa_macsec_secy_conf_t));
    memset(&tx_sc_conf_get, 0, sizeof(mepa_macsec_tx_sc_conf_t));
    memset(&sak, 0, sizeof(mepa_macsec_sak_t));
    memset(&rx_sc_conf, 0, sizeof(mepa_macsec_rx_sc_conf_t));
    uint32_t next_pn = 0;
    int xpn = 0;
    mepa_bool_t conf = 0;
    mepa_macsec_ssci_t ssci;
    mepa_bool_t active;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    switch(mreq->conf_get) {
    case MACSEC_SECY_CONF_GET:
        if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
            T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
            return;
        }
        cli_printf("\n\nSecY Configuration for port no %d with port id %d \n", req->port_no, mreq->port_id);
        cli_printf("=============================================================\n");
        cli_printf("%-25s :","Frame Validation");
        switch(secy_conf_get.validate_frames) {
        case MEPA_MACSEC_VALIDATE_FRAMES_CHECK:
            cli_printf(" Check\n");
            break;
        case MEPA_MACSEC_VALIDATE_FRAMES_STRICT:
           cli_printf(" Strict\n");
           break;
        default:
           cli_printf(" Disabled\n");
           break;
        }
        cli_printf("%-25s : %s\n","Replay protect", secy_conf_get.replay_protect ? "Enabled" : "Disabled");
        cli_printf("%-25s : %d\n","Replay Window", secy_conf_get.replay_window);
        cli_printf("%-25s : %s\n","Protect frames", secy_conf_get.protect_frames ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","always_include_sci", secy_conf_get.always_include_sci ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","use_es", secy_conf_get.use_es ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","use_scb", secy_conf_get.use_scb ? "Enabled" : "Disabled");
        cli_printf("%-25s : %d\n","confidentiality_offset", secy_conf_get.confidentiality_offset);
        cli_printf("%-25s :","Cipher suit");
        switch(secy_conf_get.current_cipher_suite) {
        case MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128:
            cli_printf(" NON-XPN AES-128\n");
            break;
        case MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256:
            cli_printf(" NON-XPN AES- 256\n");
            break;
        case MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128:
            cli_printf(" AES -XPN -128\n");
            break;
        default:
           cli_printf(" AES-XPN-256\n");
           break;
        }
        cli_printf("%-25s : 0x%x-0x%x-0x%x-0x%x-0x%x-0x%x\n","Mac Add", secy_conf_get.mac_addr.addr[0], secy_conf_get.mac_addr.addr[1],secy_conf_get.mac_addr.addr[2],
                   secy_conf_get.mac_addr.addr[3], secy_conf_get.mac_addr.addr[4], secy_conf_get.mac_addr.addr[5]);
        break;
    case MACSEC_TX_SC_CONF_GET:
        if ((rc = mepa_macsec_tx_sc_get_conf(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &tx_sc_conf_get)) != MEPA_RC_OK) {
            T_E("\n Error in getting the Tx SC Conf on port : %d \n", req->port_no);
            return;
        }
        cli_printf("Tx Secure Channel Configuration for port no %d with port id %d \n", req->port_no, mreq->port_id);
        cli_printf("=======================================================================\n");
        cli_printf("%-25s : %s\n","Protect frames", tx_sc_conf_get.protect_frames ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","always_include_sci", tx_sc_conf_get.always_include_sci ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","use_es", tx_sc_conf_get.use_es ? "Enabled" : "Disabled");
        cli_printf("%-25s : %s\n","use_scb", tx_sc_conf_get.use_scb ? "Enabled" : "Disabled");
        cli_printf("%-25s : %d\n","confidentiality_offset", tx_sc_conf_get.confidentiality_offset);
        break;
    case MACSEC_TX_SA_CONF_GET:
        /* Get Current Cipher Suit of the Secure Association */
        if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
            T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
            return;
        }
        if(secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128 || secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256)
        {
            xpn = 0;
            if ((rc = mepa_macsec_tx_sa_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no, &next_pn, &conf,&sak,&active))!=MEPA_RC_OK) {
                T_E("\n Error in getting TX Secure Association Conf on port : %d \n", req->port_no);
                return;
            }
        } else {
            xpn = 1;
            if ((rc = mepa_macsec_tx_seca_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, mreq->an_no, &next_pn_xpn, &conf, &sak, &active,
                                              &ssci))!= MEPA_RC_OK) 
            {
                T_E("\n Error in getting the Tx XPN Secure Association Get on port : %d \n", req->port_no);
                return;
            }
        }
        cli_printf("Tx Secure Association Configuration for port no %d with port id %d \n", req->port_no, mreq->port_id);
        cli_printf("=============================================================================\n");
        if(xpn) {
            cli_printf("%-25s : %ld\n","Next Pn", next_pn_xpn.xpn);
        } else {
           cli_printf("%-25s : %ld\n","Next Pn", next_pn);
        }
        cli_printf("%-25s : %s\n","confidentiality", conf ? "Enabled":"Disabled");
        cli_printf("%-25s : %s\n","Active", active ? "Yes" : "No");
        cli_printf("%-25s :", "Key");
        for(int i = 0; i < MAX_KEY_LEN; i++) {
            cli_printf(" %d", sak.buf[i]);
            if(i < MAX_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n%-25s :", "Hash Key");
        for(int i = 0; i < MAX_HASK_KEY_LEN; i++) {
            cli_printf(" %d", sak.h_buf[i]);
            if(i < MAX_HASK_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n%-25s :", "Salt");
        for(int i = 0; i < MAX_SALT_KEY_LEN; i++) {
            cli_printf(" %d", sak.salt.buf[i]);
            if(i < MAX_SALT_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n");
        break;
    case MACSEC_RX_SC_CONF_GET:

        if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
            T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
            return;
        }

        memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
        sci.port_id = mreq->rx_sc_id;
        cli_printf("Rx Secure Channel Configuration for port no %d with port id %d and channel id %d\n", req->port_no, mreq->port_id, mreq->rx_sc_id);
        cli_printf("=================================================================================================\n");
        if ((rc = mepa_macsec_rx_sc_get_conf(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, &rx_sc_conf)) != MEPA_RC_OK) {
            T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
            return;
        }
        cli_printf("%-25s :","Frame Validation");
        switch(rx_sc_conf.validate_frames) {
        case MEPA_MACSEC_VALIDATE_FRAMES_CHECK:
            cli_printf(" Check\n");
            break;
        case MEPA_MACSEC_VALIDATE_FRAMES_STRICT:
           cli_printf(" Strict\n");
           break;
        default:
           cli_printf(" Disabled\n");
           break;
        }
        cli_printf("%-25s : %s\n","Replay protect", rx_sc_conf.replay_protect ? "Enabled" : "Disabled");
        cli_printf("%-25s : %d\n","Replay Window", rx_sc_conf.replay_window);
        cli_printf("%-25s : %d\n","confidentiality_offset", rx_sc_conf.confidentiality_offset);
        break;
    case MACSEC_RX_SA_CONF_GET:

        if ((rc = mepa_macsec_secy_conf_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf_get)) != MEPA_RC_OK) {
            T_E("\n Error in getting the SecY on port : %d \n", req->port_no);
            return;
        }

        memcpy(&sci.mac_addr, &secy_conf_get.mac_addr, sizeof(mepa_mac_t));
        sci.port_id = mreq->rx_sc_id;

        if(secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_128 || secy_conf_get.current_cipher_suite == MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256)
        {
            xpn = 0;
            if ((rc = mepa_macsec_rx_sa_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci, mreq->an_no,&next_pn,&sak,&active)) != MEPA_RC_OK) {
                T_E("\n Error in getting the Rx SA Conf on port : %d \n", req->port_no);
                return;
            }
        } else {
            xpn = 1;
            if ((rc = mepa_macsec_rx_seca_get(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &sci,mreq->an_no, &next_pn_xpn, &sak, &active,
                                              &ssci)) != MEPA_RC_OK)
            {
                T_E("\n Error in getting the Rx SA Conf on port : %d \n", req->port_no);
                return;
            }
        }
        cli_printf("Rx Secure Channel Configuration for port no %d with port id %d and channel id %d and an = %d\n", req->port_no, mreq->port_id, mreq->rx_sc_id,
                   mreq->an_no);
        cli_printf("====================================================================================================================\n");
        if(xpn) {
            cli_printf("%-25s : %ld\n","Lowest Pn", next_pn_xpn.xpn);
        } else {
            cli_printf("%-25s : %ld\n","Lowest Pn", next_pn);
        }
        cli_printf("%-25s : %s\n","Active", active ? "Yes" : "No");
        cli_printf("%-25s :", "Key");
        for(int i = 0; i < MAX_KEY_LEN; i++) {
            cli_printf(" %d", sak.buf[i]);
            if(i < MAX_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n%-25s :", "Hash Key");
        for(int i = 0; i < MAX_HASK_KEY_LEN; i++) {
            cli_printf(" %d", sak.h_buf[i]);
            if(i < MAX_HASK_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n%-25s :", "Salt");
        for(int i = 0; i < MAX_SALT_KEY_LEN; i++) {
            cli_printf(" %d", sak.salt.buf[i]);
            if(i < MAX_SALT_KEY_LEN - 1)
                cli_printf(",");
        }
        cli_printf("\n");
        break;
    default:
        break;
    }
    return;
}

static void cli_cmd_cltr_frame_set(cli_req_t *req)
{
    mepa_rc rc;
    int match = 0;
    char input[3];
    macsec_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    mepa_macsec_control_frame_match_conf_t conf;
    cli_printf("\n");
    cli_printf("\n 1 = DMAC Pattern Matching");
    cli_printf("\n 2 = Ethertype Patter Matching");
    cli_printf("\n 3 = DMAC and Ethertype Match");
    cli_printf("\n\n Parameter that needs to be Matched : ");
    scanf("%s", &input[0]);
    match = atoi_Conversion(input);
    cli_printf("\n");
    switch(match) {
    case 1:
        conf.match = MEPA_MACSEC_MATCH_DMAC;
        break;
    case 2:
        conf.match = MEPA_MACSEC_MATCH_ETYPE;
        break;
    case 3:
        conf.match = MEPA_MACSEC_MATCH_DMAC | MEPA_MACSEC_MATCH_ETYPE;
        break;
    default:
        T_E("\n Invalid Match Input\n");
        return;
    }
    memcpy(&conf.dmac, &mreq->pattern_dst_mac_addr, sizeof(mreq->pattern_dst_mac_addr));
    conf.etype = mreq->pattern_ethtype;

    if ((rc = mepa_macsec_control_frame_match_conf_set(meba_macsec_instance->phy_devices[req->port_no], req->port_no, &conf, NULL)) != MEPA_RC_OK) {
        T_E("\n Error in Configuring Control Match Configuration on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n ...... Control Frame Match Configured on port : %d  ......\n", req->port_no);
    return;
}

static void cli_cmd_cltr_frame_get(cli_req_t *req)
{
    mepa_rc rc;
    uint32_t max_rules = 0;
    demo_phy_info_t phy_family;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    if ((rc = phy_family_detect(meba_macsec_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    max_rules = (phy_family.family == PHY_FAMILY_MALIBU_25G) ? LAN80XX_MAX_CP_RULES : VSC_PHY_MAX_CP_RULES;
    mepa_macsec_control_frame_match_conf_t conf;
    cli_printf("\n\n Rules configured for 802.1X Control Traffic Bypass on Port %d", req->port_no);
    cli_printf("\n==================================================================================================\n");
    cli_printf("\n %-10s%-15s%-30s%s", "Rule id", "Ethertype", "DMAC Address", "Match");
    cli_printf("\n---------------------------------------------------------------------------------\n");
    for(uint32_t i = 0; i < max_rules; i++) {
        if ((rc = mepa_macsec_control_frame_match_conf_get(meba_macsec_instance->phy_devices[req->port_no], req->port_no, &conf, i)) != MEPA_RC_OK) {
            T_E("\n Error in Geting Control Match Configuration on port : %d \n", req->port_no);
            return;
        }
        if(conf.match > 1) {
            cli_printf("\n %-10d%-15x%02x-%02x-%02x-%02x-%02x-%02x", i, conf.etype, conf.dmac.addr[0], conf.dmac.addr[1], conf.dmac.addr[2],
                      conf.dmac.addr[3], conf.dmac.addr[4], conf.dmac.addr[5]); 
            if(conf.match == MEPA_MACSEC_MATCH_DMAC) {
                cli_printf("   %s\n", "DMAC");
            } else if(conf.match == MEPA_MACSEC_MATCH_ETYPE) {
                cli_printf("   %s\n", "Ethtype");
            } else if(conf.match == (MEPA_MACSEC_MATCH_ETYPE | MEPA_MACSEC_MATCH_DMAC)) {
                cli_printf("   %s\n", "DMAC and Ethtype");
            }
         }
    }
    return;
}

static void cli_cmd_cltr_frame_del(cli_req_t *req)
{
    mepa_rc rc;
    macsec_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    for(int i = 0; i < mreq->value_cnt; i++) {
        if ((rc = mepa_macsec_control_frame_match_conf_del(meba_macsec_instance->phy_devices[req->port_no], req->port_no, mreq->value_list[i])) != MEPA_RC_OK) {
            T_E("\n Error in Geting Control Match Configuration on port : %d \n", req->port_no);
            return;
        }
    }
    return;
}

static void cli_cmd_macsec_inst_get(cli_req_t *req)
{
    mepa_rc rc;

    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }

        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", port_no);
            return;
        }

        mepa_macsec_inst_count_t inst_get;
        if ((rc = mepa_macsec_inst_count_get(meba_macsec_instance->phy_devices[iport], iport, &inst_get)) != MEPA_RC_OK) {
            T_E("\n Error in Getting MACsec Instances on the Port : %d \n", port_no);
            return;
        }
        cli_printf("\n\n");
        cli_printf("MACsec Instance Statisitics on port no %d\n", iport);
        cli_printf("=============================================================\n");
        cli_macsec_statistics("Number of SecYs","NULL", inst_get.no_secy, 0);
        cli_printf("%-30s %s", "SecY Port ids", ":");
        for(int i = 0; i < inst_get.no_secy; i++) {
            cli_printf(" %d,", inst_get.secy_vport[i]);
        }
        cli_printf("\n\n");
        cli_printf("SecY id   No of Txsc   No of TxSA   TxSA Id            No of Rx SC    Rx SC Ids(No of SA in SC)");
        cli_printf("\n=================================================================================================\n");
        if(inst_get.no_secy == 0) {
            cli_printf("\n  --         --           --          --                   --                  --\n");
        }
        for(int i = 0; i < inst_get.no_secy; i++) {
            cli_printf("%    -12d%-13d%-13d", inst_get.secy_vport[i], inst_get.secy_inst_count[i].no_txsc, inst_get.secy_inst_count[i].txsc_inst_count.no_sa);
            if(inst_get.secy_inst_count[i].txsc_inst_count.no_sa == 0) {
                cli_printf("%-20s", "--");
            }
            for(int k = 0; k < inst_get.secy_inst_count[i].txsc_inst_count.no_sa; k++) {
                cli_printf(" %d,",inst_get.secy_inst_count[i].txsc_inst_count.sa_id[k]);
                if(k == (inst_get.secy_inst_count[i].txsc_inst_count.no_sa - 1)) {
                    for(int j = 0; j < (20 - (3 *inst_get.secy_inst_count[i].txsc_inst_count.no_sa)); j++) {
                        cli_printf(" ");
                    }
                }
            }
            cli_printf("%-11d", inst_get.secy_inst_count[i].no_rxsc);
            for(int j = 0; j < inst_get.secy_inst_count[i].no_rxsc; j++) {
                cli_printf(" %d (%d),", inst_get.secy_inst_count[i].rx_sci[j].port_id, inst_get.secy_inst_count[i].rxsc_inst_count[j].no_sa);
            }
            if(inst_get.secy_inst_count[i].no_rxsc == 0) {
                cli_printf(" ---------");
            }
            cli_printf("\n");
        }
        cli_printf("\n");
    }
    return;
}

static void cli_cmd_macsec_cmds()
{
    cli_printf("\n\n");
    cli_printf("\n\033[1;32m%-20s%-80s%s\033[0m\n","  Commands", " Arguments","  Description");
    cli_printf("\033[1;32m==================================================================================================================================\033[0m\n");
    cli_printf("\n %-20s| %-80s| %s", "exit_macsec", "", "Exit from MACsec Application");
    cli_printf("\n %-20s| %-80s| %s", "macsec port_state", "", "Provides MACsec Capability and MACsec State of all Ports");
    cli_printf("\n %-20s| %-80s| %s", "macsec enable", " <port_list> [bypass_none|bypass_disable]", "Enables MACsec Block");
    cli_printf("\n %-20s| %-80s| %s", "macsec disable", "<port_list> [bypass_none|bypass_enable]", "Disables MACsec Block");
    cli_printf("\n %-20s| %-80s| %s", "secy_create_upd", "[create|update] <port_no> port-id <port_id> mac-addr <mac_address>", "Create/Update Secure Entity");
    cli_printf("\n %-20s| %-80s| %s", "secy_del", "<port_no> port-id <port_id>", "Deletes available Secure Entity");
    cli_printf("\n %-20s| %-80s| %s", "match_conf", "<port_no> ethtype <ether_type> src_mac <mac_address> dst_mac <mac_address>", "Pattern Matching Params");
    cli_printf("\n %-20s| %-80s| %s", " ", "vlanid <vlan> vid-inner <vlan>", " ");
    cli_printf("\n %-20s| %-80s| %s", "pattern_conf", "<port_no> port-id <port_id> [egress|ingress] [cont_port|uncont_port|drop]", " ");
    cli_printf("\n %-20s| %-80s| %s", "tx_sc_create", "<port_no> port-id <port_id>", "Transmit Secure Channel Create");
    cli_printf("\n %-20s| %-80s| %s", "rx_sc_create", "<port_no> port-id <port_id> sc-id <id>", "Receive Secure Channel Create");
    cli_printf("\n %-20s| %-80s| %s", "tx_sc_del", "<port_no> port-id <port_id>", "transmit Secure Channel Delete");
    cli_printf("\n %-20s| %-80s| %s", "rx_sc_del", "<port_no> port-id <port_id> sc-id <id>", "Receive Secure Channel Delete");
    cli_printf("\n %-20s| %-80s| %s", "tx_sa_create", "<port_no> port-id <port_id> an <an_no> next-pn <pn> conf [true|false]", "Transmit Secure Association Create");
    cli_printf("\n %-20s| %-80s| %s", "rx_sa_create", "<port_no> port-id <port_id> sc-id <id> an <an_no> lowest-pn <pn>", "Receive Secure Association Create");
    cli_printf("\n %-20s| %-80s| %s", "tx_sa_del", "<port_no> port-id <port_id> an <an_no>", "Transmit Secure Association Delete");
    cli_printf("\n %-20s| %-80s| %s", "rx_sa_del", "<port_no> port-id <port_id> sc-id <id> an <an_no>", "Receive Secure Association Delete");
    cli_printf("\n %-20s| %-80s| %s", "rx_sa_pn_upd", "<port_no> port-id <port_id> sc-id <id> an <an_no> lowest-pn <pn>", "Receive SA Lowest PN Update");
    cli_printf("\n %-20s| %-80s| %s", "statistics secy", "<port_no> port-id <port_id> [get|clear]", "SecY Staticstics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "statistics tx_sc", "<port_no> port-id <port_id> [get|clear]", "Tx Secure Channel Statistics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "statistics tx_sa", "<port_no> port-id <port_id> an <an_no> [get|clear]", "Tx Secure Association Statistics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "statistics rx_sc", "<port_no> port-id <port_id> sc-id <id> [get|clear]", "Rx Secure Channel Statistics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "statistics rx_sa","<port_no> port-id <port_id> sc-id <id> an <an_no> [get|clear]","Rx SA Statistics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "statistics mac", "[hmac|lmac] <port_no> [get|clear]", "HMAC/LMAC Statistics Get or Clear");
    cli_printf("\n %-20s| %-80s| %s", "vlan_bypass", "<port_no> port-id <port_id> [zero|one|two|three|four]", "Vlan Tag Bypass Configuration");
    cli_printf("\n %-20s| %-80s| %s", "frame capture", "<port_list> [egress|ingress]", "MACsec FIFO Frame Capture Configuration");
    cli_printf("\n %-20s| %-80s| %s", "frame get", "<port_list>", "MACsec FIFO Captured Frame Get");
    cli_printf("\n %-20s| %-80s| %s", "macsec event set", "<port_list> [rollover|seq_threshold] [evt_enable|evt_disable]", "MACsec Event Enable or Disable");
    cli_printf("\n %-20s| %-80s| %s", "macsec event get", "", "MACsec Event Status of all Ports");
    cli_printf("\n %-20s| %-80s| %s", "macsec seq_threshold", "[get|set] <port_list> [<threshold_val>]", "MACsec Sequence Thresold Val Set or Get");
    cli_printf("\n %-20s| %-80s| %s", "ctrl_frame set", "<port_no> ethtype <ether_type> dst_mac <mac_address>", "802.1X Control Traffic Bypass Confg Set");
    cli_printf("\n %-20s| %-80s| %s", "ctrl_frame get", "<port_no>", "802.1X Control Traffic Bypass Confg Get");
    cli_printf("\n %-20s| %-80s| %s", "ctrl_frame del", "<port_no> <rule_list>", "802.1X Control Traffic Bypass Confg Del");
    cli_printf("\n %-20s| %-80s| %s", "conf_get", "[secy|tx_sc|tx_sa|rx_sc|rx_sa] <port_no> port-id <port_id> sc-id <id> an <an_no>", "Get MACsec Configurations");
    cli_printf("\n\n");
    return;
}

static void cli_cmd_macsec_demo(cli_req_t *req)
{
    cli_printf("\n\t\t\t\t \033[1;31m ================================================================================================================\033[0m\n");
    cli_printf("\t\t\t\t\t  \t\t\t\t \033[1;31mMACSEC DEMO APPLICATION\033[0m\n");
    cli_printf("\n\t\t\t\t  \033[1;31m================================================================================================================\033[0m\n");
    cli_printf("\n\n");
    cli_cmd_macsec_cmds();
    return;
}


static int cli_param_parse_bypass_config(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_BYPASS_NONE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_NONE;
    }
    else if (!strncasecmp(found, KEYWORD_BYPASS_DISABLE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_DISABLE;
    }
    else if (!strncasecmp(found, KEYWORD_BYPASS_ENABLE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_ENABLE;
    }
    return 0;
}


static int cli_cmd_parse_u16_param(cli_req_t *req)
{
    uint16_t value;
    macsec_configuration *mreq = req->module_req;
    int error = 0;

    if (keyword.etype_parsed == 1) {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->pattern_ethtype = value;
        keyword.etype_parsed = 0;
    } else if (keyword.vlan_id_parsed == 1) {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->vid = value;
        keyword.vlan_id_parsed = 0;
    } else if (keyword.vlan_inner_id_parsed == 1) {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->vid_inner = value;
        keyword.vlan_inner_id_parsed = 0;
    } else if (keyword.rx_sc_id_parsed == 1) {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->rx_sc_id = value;
        keyword.rx_sc_id_parsed = 0;
    } else if (keyword.an_parsed == 1) {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->an_no = value;
        keyword.an_parsed = 0;
    } else {
        error = cli_parm_u16(req, &value, 0, MASK_16BIT);
        mreq->port_id = value;
    }
    return error;
}

static int cli_cmd_parse_boolean(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    if (!strncasecmp(req->cmd, KEYWORD_CREATE, strlen(req->cmd))) {
        mreq->secy_create = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_UPDATE, strlen(req->cmd))) {
        mreq->secy_create = 0;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_TRUE, strlen(req->cmd))) {
        mreq->conf = 1;
        mreq->protect_frames = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_FALSE, strlen(req->cmd))) {
        mreq->conf = 0;
        mreq->protect_frames = 0;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_EGRESS, strlen(req->cmd))) {
        mreq->egress_direction = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_INGRESS, strlen(req->cmd))) {
        mreq->egress_direction = 0;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_HMAC, strlen(req->cmd))) {
        mreq->hmac_counters = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_LMAC, strlen(req->cmd))) {
        mreq->hmac_counters = 0;
    }
    return 0;
}

static int cli_cmd_parse_mac_addr(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    uint32_t  i, mac[6];
    int error = 1;
    int len = 0;
    len = strlen(req->cmd);
    if(len > MAC_ADDRESS_LEN) {
        cli_printf("\n Invalid MAC Address argument\n");
	return 1;
    }
    if (sscanf(req->cmd, "%2x-%2x-%2x-%2x-%2x-%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
        if (keyword.dst_mac_parsed == 1) {
            for (i = 0; i < 6; i++) {
                mreq->pattern_dst_mac_addr.addr[i] = (mac[i] & 0xff);
            }
	    keyword.dst_mac_parsed = 0;
        }
	else if (keyword.src_mac_parsed == 1) {
            for (i = 0; i < 6; i++) {
                mreq->pattern_src_mac_addr.addr[i] = (mac[i] & 0xff);
             }
	     keyword.src_mac_parsed = 0;
        }
        else {
            for (i = 0; i < 6; i++) {
                mreq->mac_addr.addr[i] = (mac[i] & 0xff);
            }
        }
        error = 0;
    }
    else {
        cli_printf("\n Invalid MAC Address argument\n");
    }
    return error;
}

static int cli_cmd_parse_match_action(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    if (!strncasecmp(req->cmd, KEYWORD_CONT_PORT, strlen(req->cmd))) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_CONTROLLED_PORT;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_UNCONT_PORT, strlen(req->cmd))) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_DROP, strlen(req->cmd))) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_DROP;
    }
    return 0;
}

static int cli_cmd_parse_keyword(cli_req_t *req)
{
    if (!strncasecmp(req->cmd, KEYWORD_ETHTYPE, strlen(req->cmd))) {
        keyword.etype_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_SRC_MAC, strlen(req->cmd))) {
        keyword.src_mac_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_DST_MAC, strlen(req->cmd))) {
        keyword.dst_mac_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLANID, strlen(req->cmd))) {
        keyword.vlan_id_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VIDINNER, strlen(req->cmd))) {
        keyword.vlan_inner_id_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_SC_ID, strlen(req->cmd))) {
        keyword.rx_sc_id_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_AN, strlen(req->cmd))) {
        keyword.an_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_NEXT_PN, strlen(req->cmd))) {
        keyword.next_pn_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_CONF, strlen(req->cmd))) {
        keyword.conf_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_LOW_PN, strlen(req->cmd))) {
        keyword.lowest_pn_parsed = 1;
    }
    return 0;
}


static int cli_cmd_parse_u64_param(cli_req_t *req)
{
    uint64_t value;
    macsec_configuration *mreq = req->module_req;
    int error = 0;
    if (keyword.next_pn_parsed == 1) {
        error = cli_parm_u64(req, &value, 0, MASK_64BIT);
        mreq->next_pn = value;
        keyword.next_pn_parsed = 0;
    }

    else if (keyword.lowest_pn_parsed == 1) {
        error = cli_parm_u64(req, &value, 0, MASK_64BIT);
        mreq->lowest_pn = value;
        keyword.lowest_pn_parsed = 0;
    } else {
        error = cli_parm_u64(req, &value, 0, MASK_64BIT);
        mreq->seq_threshold_value = value;
    }

    return error;
}

static int cli_cmd_stats_get_clear(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;

    if (!strncasecmp(req->cmd, KEYWORD_GET, strlen(req->cmd))) {
        mreq->statistics_get = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_CLEAR, strlen(req->cmd))) {
        mreq->statistics_get = 0;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_SET, strlen(req->cmd))) {
        mreq->statistics_get = 0;
    }

    return 0;
}

static int cli_cmd_parse_vlan(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    if (!strncasecmp(req->cmd, KEYWORD_VLAN_ZERO, strlen(req->cmd))) {
        mreq->vlan_tag = 0;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLAN_ONE, strlen(req->cmd))) {
        mreq->vlan_tag = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLAN_TWO, strlen(req->cmd))) {
        mreq->vlan_tag = 2;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLAN_THREE, strlen(req->cmd))) {
        mreq->vlan_tag = 3;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLAN_FOUR, strlen(req->cmd))) {
        mreq->vlan_tag = 4;
    }
    return 0;
}

static int cli_cmd_event_ena_dis(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;

    if (!strncasecmp(req->cmd, KEYWORD_EVT_ENABLE, strlen(req->cmd))) {
        mreq->enable = 1;
    } else {
        mreq->enable = 0;
    }
    return 0;
}

static int cli_cmd_event_parse(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    
    if (!strncasecmp(req->cmd, KEYWORD_ROLLOVER_EVT, strlen(req->cmd))) {
        mreq->rollover_event = 1;
    } else {
        mreq->rollover_event = 0;
    }

    if (!strncasecmp(req->cmd, KEYWORD_SEQ_THRE_EVT, strlen(req->cmd))) {
        mreq->sequence_threshold_event = 1;
    } else {
        mreq->sequence_threshold_event = 0;
    }
    return 0;
}

static int cli_cmd_macsec_param_conf_get(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;

    if (!strncasecmp(req->cmd, KEYWORD_SECY_GET, strlen(req->cmd))) {
        mreq->conf_get = MACSEC_SECY_CONF_GET;
    } else if (!strncasecmp(req->cmd, KEYWORD_TX_SC_GET, strlen(req->cmd))) {
        mreq->conf_get = MACSEC_TX_SC_CONF_GET;
    } else if (!strncasecmp(req->cmd, KEYWORD_TX_SA_GET, strlen(req->cmd))) {
        mreq->conf_get = MACSEC_TX_SA_CONF_GET;
    } else if (!strncasecmp(req->cmd, KEYWORD_RX_SC_GET, strlen(req->cmd))) {
        mreq->conf_get = MACSEC_RX_SC_CONF_GET;
    } else if (!strncasecmp(req->cmd, KEYWORD_RX_SA_GET, strlen(req->cmd))) {
        mreq->conf_get = MACSEC_RX_SA_CONF_GET;
    } else {
        mreq->conf_get = MACSEC_CONF_GET_NONE;
    }
    return 0;
}
static int cli_parm_value_rules(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    return cli_parse_values(req->cmd, mreq->value_list ,&mreq->value_cnt, 0, 26, 30);
}

static cli_cmd_t cli_cmd_macsec_table[] = {
    {
        "exit_macsec",
        "Exit MACsec Demo Application",
        cli_cmd_macsec_demo,
    },

    {
        "?",
        "Lists the Available MACsec Commands",
         cli_cmd_macsec_cmds,
    }, 

    {
        "macsec enable <port_list> [bypass_none|bypass_disable]",
        "Enable the MACsec Block",
         cli_cmd_macsec_ena,
    },

    {
        "macsec disable <port_list> [bypass_none|bypass_enable]",
        "Disable the MACsec Block",
         cli_cmd_macsec_dis,
    },

    {
        "macsec port_state",
        "Status of MACsec Engine on all Ports",
         cli_cmd_macsec_port_state,
    },

    {
        "secy_create_upd [create|update] <port_no> port-id <port_id> mac-addr <mac_address>",
        "Create or Update the MACsec Secure Entity",
        cli_cmd_secy_create,
    },

    {
        "secy_del <port_no> port-id <port_id>",
        "Delete the Secure Entity",
        cli_cmd_macsec_secy_del,
    },    

    {
        "match_conf <port_no> ethtype <ether_type> src_mac <mac_address> dst_mac <mac_address> vlanid <vlan> vid-inner <vlan>",
        "Pattern Match Parameters",
        cli_cmd_match_param,
    },

    {
        "pattern_conf <port_no> port-id <port_id> [egress|ingress] [cont_port|uncont_port|drop]",
        "Pattern Match Configuration",
        cli_cmd_match_conf,
    },

    {
        "tx_sc_create <port_no> port-id <port_id>",
        "Transmit Secure Channel Creation",
        cli_cmd_tx_sc_create,
    },

    {
        "rx_sc_create <port_no> port-id <port_id> sc-id <id>",
        "Receive Secure Channel Creation",
        cli_cmd_rx_sc_create,
    },

    {
        "tx_sc_del <port_no> port-id <port_id>",
        "Delete Transmit Secure Channel",
        cli_cmd_macsec_tx_sc_del,
    },

    {
        "rx_sc_del <port_no> port-id <port_id> sc-id <id>",
        "Delete Receive Secure Channel",
        cli_cmd_macsec_rx_sc_del,
    },

    {
        "tx_sa_create <port_no> port-id <port_id> an <an_no> next-pn <pn> conf [true|false]",
        "Transmit Secure Assosiation Creation",
        cli_cmd_tx_sa_create,
    },

    {
        "rx_sa_create <port_no> port-id <port_id> sc-id <id> an <an_no> lowest-pn <pn>",
        "Receive Secure Assosiation Creation",
        cli_cmd_rx_sa_create,
    },

    {
        "tx_sa_del <port_no> port-id <port_id> an <an_no>",
        "Delete the Transmit Secure Association",
        cli_cmd_macsec_tx_sa_del,
    },

    {
        "rx_sa_del <port_no> port-id <port_id> sc-id <id> an <an_no>",
        "Delete Receive Secure Association",
        cli_cmd_macsec_rx_sa_del,
    },

    {
        "rx_sa_pn_upd <port_no> port-id <port_id> sc-id <id> an <an_no> lowest-pn <pn>",
        "Receive Secure Assosiation Creation",
        cli_cmd_rx_sa_pn_update,
    },

    {
        "statistics secy <port_no> port-id <port_id> [get|clear]",
        "Get or Clear the Secure Entity Statistics",
         cli_cmd_macsec_secy_statistics,
    },

    {
        "statistics tx_sc <port_no> port-id <port_id> [get|clear]",
        "Get or Clear Transmit Secure channel Statistics",
        cli_cmd_macsec_tx_sc_statistics,
    },

    {
        "statistics tx_sa <port_no> port-id <port_id> an <an_no> [get|clear]",
        "Get or Clear Transmit Secure Association Statistics",
        cli_cmd_macsec_tx_sa_statistics,
    },

    {
        "statistics rx_sc <port_no> port-id <port_id> sc-id <id> [get|clear]",
        "Get or Clear Receive Secure channel Statistics",
        cli_cmd_macsec_rx_sc_statistics,
    },

    {
        "statistics rx_sa <port_no> port-id <port_id> sc-id <id> an <an_no> [get|clear]",
        "Get or Clear Receive Secure Association Statistics",
        cli_cmd_macsec_rx_sa_statistics,
    },

    {
        "statistics mac [hmac|lmac] <port_no> [get|clear]",
        "Get or Clear HOST/LINE MAC Statistics",
        cli_cmd_macsec_mac_statistics,
    },

    {
        "vlan_bypass <port_no> port-id <port_id> [zero|one|two|three|four]",
        "Configures the Number of VLAN needs to be Bypassed",
        cli_cmd_macsec_vlan_bypass,
    },

    {
        "frame capture <port_list> [egress|ingress]",
        "Enable MACsec FIFO to Capture the Frame",
        cli_cmd_macsec_frame_capt,
    },

    {
        "frame get <port_list>",
        "Read the frame captured in MACsec FIFO",
        cli_cmd_macsec_frame_get,
    },

    {
        "macsec event set <port_list> [rollover|seq_threshold] [evt_enable|evt_disable]",
        "MACsec Event Configuration",
        cli_cmd_macsec_event_set,
    },

    {
        "macsec event get",
        "Get the Status of MACsec Events",
        cli_cmd_macsec_event_get,
    },

    {
        "macsec seq_threshold [get|set] <port_list> [<threshold_val>]",
        "Set/get Packet Number threshold value for MACsec Event",
        cli_cmd_macsec_seq_num_set,
    },

    {
        "conf_get [secy|tx_sc|tx_sa|rx_sc|rx_sa] <port_no> port-id <port_id> sc-id <id> an <an_no>",
        "Get the Conf of SecY/SC/SA",
         cli_cmd_macsec_conf_get,
    },

    {
        "ctrl_frame set <port_no> ethtype <ether_type> dst_mac <mac_address>",
        "802.1X Control Traffic Bypass Configuration",
         cli_cmd_cltr_frame_set,
    },

    {
        "ctrl_frame get <port_no>",
        "Configurations for 802.1X Control Traffic Bypass",
         cli_cmd_cltr_frame_get,
    },

    {
        "ctrl_frame del <port_no> <rule_list>",
        "Delete the Configured rule for 802.1X Control Traffic bypass",
        cli_cmd_cltr_frame_del,
    },

    {
        "inst_get <port_list>",
        "Indicates number of SecY,SC,SAs created on Physical port",
        cli_cmd_macsec_inst_get,
    }

};


static cli_cmd_t cli_cmd_table[] = {
    {
        "macsec",
        "MACsec Operational Demo",
        cli_cmd_macsec_demo,
    },
};

static cli_parm_t cli_parm_table[] = {

    {
        "bypass_none|bypass_disable",
        "\n\n\t bypass_none      : MAC block and MACsec Block Configuration \n"
        "\t bypass_disable   : Enable MACsec Block from Bypass \n",
        CLI_PARM_FLAG_SET,
        cli_param_parse_bypass_config,
    },

    {
        "bypass_none|bypass_enable",
        "\n\n\t bypass_none      : MAC block and MACsec Block Configuration \n"
        "\t bypass_enable    : Put MACsec Block in Bypass \n",
        CLI_PARM_FLAG_SET,
        cli_param_parse_bypass_config,
    },


    {
        "<port_id>",
        "Secy identifier",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "<id>",
        "Receive Secure Channel Identifier",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "create|update",
        "\n\t create    : Create new MACsec Secure Entity \n"
        "\t update    : Update the Configuration of existing Secure Entity \n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_boolean,
    },

    {
        "<mac_address>",
        "MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_mac_addr,
    },

    {
        "<ether_type>",
        "Ether-type of Packet",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "egress|ingress",
        "Traffic Direction Egress or Ingress",
         CLI_PARM_FLAG_SET,
         cli_cmd_parse_boolean,
    },

    {
        "cont_port|uncont_port|drop",
        "Forward Traffic to Controlled/Uncrontrolled Port or Drop Packet",
         CLI_PARM_FLAG_SET,
         cli_cmd_parse_match_action,
    },

    {
        "get|clear",
        "\n\t get : Get the Statistics\n"
        "\t clear : Clear the Statistics\n",
        CLI_PARM_FLAG_SET,
        cli_cmd_stats_get_clear,
    },

    {
        "hmac|lmac",
        "\n\t hmac : Host MAC Statistics\n"
        "\t lmac : Line MAC Statistics\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_boolean,
    },

    {
        "zero|one|two|three|four",
        "\n\t zero : Disable VLAN Bypass\n"
        "\t one   : Bypass one VLAN Tag\n"
        "\t two   : Bypass two VLAN Tag\n"
        "\t three : Bypass three VLAN Tag\n"
        "\t four  : Bypass four VLAN Tag\n",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_vlan,
    },

    {
        "true|false",
        "Enable or Disable",
         CLI_PARM_FLAG_SET,
         cli_cmd_parse_boolean,
    },

    {
        "secy|tx_sc|tx_sa|rx_sc|rx_sa",
        "\n\t secy  : SecY Configuration\n"
        "\t tx_sc : Transmit Secure Channel Configuration\n"
        "\t tx_sa : Transmit Secure Association Configuration\n"
        "\t rx_sc : Receive Secure Channel Configuration\n"
        "\t rx_sa : Receive Secure Association Configuration\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_macsec_param_conf_get,
    },

    {
        "<vlan>",
        "\n\t Vlan Id\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "<an_no>",
        "Association Number",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "<pn>",
        "Packet Number",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_u64_param,
    },

    {
        "true|false",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_boolean,
    },

    {
        "rollover|seq_threshold",
        "\n\t rollover : PN Rollover Event\n"
        "\t seq_threshold : PN Threshold event\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_event_parse,
    },

    {
        "evt_enable|evt_disable",
        "\n\t evt_enable : Enable Event\n"
        "\t evt_disable: Disable Event\n",
        CLI_PARM_FLAG_SET,
        cli_cmd_event_ena_dis,
    },

    {
        "get|set",
        "\n\t get : Get the Value\n"
        "\t set : Set the Value\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_stats_get_clear,
    },

    {
        "<threshold_val>",
        "Packet Number",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_u64_param,
    },

    {
        "<rule_list>",
        "Control Traffic Rules",
        CLI_PARM_FLAG_SET,
        cli_parm_value_rules,
    },
	
    {
        "port-id",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "mac-addr",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "src_mac",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "dst_mac",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "ethtype",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "vlanid",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "vid-inner",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "sc-id",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "an",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "next-pn",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "conf",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

     {
        "lowest-pn",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },
};


static void phy_cli_init(void)
{
    int i;

    /* Register CLI Commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

    /* Register MACsec Commands */
    for (i = 0; i < sizeof(cli_cmd_macsec_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_macsec_cli_cmd_reg(&cli_cmd_macsec_table[i]);
    }

    /* Register CLI Params */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mepa_demo_appl_macsec_demo(mscc_appl_init_t *init)
{
    meba_macsec_instance = init->board_inst;
    switch(init->cmd) {
    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;
    case MSCC_INIT_CMD_POLL:
        macsec_poll(init->board_inst);
        break;
    default:
        break;
    }
}
