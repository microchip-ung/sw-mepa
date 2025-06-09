// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "microchip/ethernet/phy/api/types.h"
#include "mesa-rpc.h"
#include "main.h"
#include "cli.h"
#include "port.h"
#include "trace.h"
#include "mepa_driver.h"
#include "phy_demo_apps.h"
#include "phy_port_config.h"

/* Supported PHY Family: Malibu 25G*/


json_object **json_items; // Array to hold JSON objects
int array_len; // Length of JSON array
int *gaphy_family = NULL;

meba_inst_t meba_phy_inst_wr; // MEBA PHY instance
static mepa_callout_t mepa_callout_wr; // Callout structure
static mepa_board_conf_t board_conf = {};

// Function prototype
static void cli_cmd_phy_warmstart_enable(cli_req_t *req);
static void cli_cmd_phy_warmstart_perform(cli_req_t *req);

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_warmstart"
};

enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};


static mscc_appl_trace_group_t trace_groups[10] = {
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

static cli_cmd_t cli_cmd_table[] = {
    {
        "phy warmstart enable",
        "Enable Warm start for PHY",
        cli_cmd_phy_warmstart_enable,
    },
    {
        "phy warmstart perform",
        "Store all the PHY configurations (port,TS,MACsec) in .json and Load back",
        cli_cmd_phy_warmstart_perform,
    }
};

/*
Function: phy_assign_callout
Assign callout functions for PHY operations
*/
static int phy_assign_callout(void)
{
    mepa_callout_wr.mmd_read = meba_mmd_read;
    mepa_callout_wr.mmd_read_inc = meba_mmd_read_inc;
    mepa_callout_wr.mmd_write = meba_mmd_write;
    mepa_callout_wr.miim_read = meba_miim_read;
    mepa_callout_wr.miim_write = meba_miim_write;
    mepa_callout_wr.lock_enter = meba_phy_inst_wr->iface.lock_enter;
    mepa_callout_wr.lock_exit = meba_phy_inst_wr->iface.lock_exit;
    mepa_callout_wr.mem_alloc = mem_alloc;
    mepa_callout_wr.mem_free = mem_free;
    mepa_callout_wr.spi_read = mepa_phy_spi_read;
    mepa_callout_wr.spi_write = mepa_phy_spi_write;
    return MEPA_RC_OK;
}

/*
Function: json_to_struct
Convert JSON Object to PHY struct
*/
int json_to_struct(json_rpc_req_t *json_req, json_object *jobj, phy25g_phy_state_t *phy25g_data)
{

    if (json_rpc_get_phy25g_phy_state_t(json_req, jobj, phy25g_data) != MESA_RC_OK) {
        T_E("Error in the json configuration");
        return 1;
    }
    return 0;
}

int process_json_items(struct mepa_device *dev, mepa_port_no_t port_no)
{
    json_rpc_req_t json_req = {};
    if (json_to_struct(&json_req, json_items[port_no], dev->data) != 0) {
        T_E("Error converting json to struct");
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}


int load_phy_data()
{
    char phy_data_bkupfile[100];
    int size;
    char *buffer = NULL;
    struct json_object *jobj;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    snprintf(phy_data_bkupfile, sizeof(phy_data_bkupfile), "/root/mepa_scripts/phy_data_ws.json");
    FILE *fp = fopen(phy_data_bkupfile, "r");
    if (fp == NULL) {
        return MEPA_RC_ERROR;
    }
    if (!(fseek(fp, 0, SEEK_END)) && ftell(fp)) {
        size = ftell(fp);
        buffer = (char *)malloc(size);
        if (buffer == NULL) {
            fclose(fp);
            return MEPA_RC_ERROR;
        }
        fseek(fp, 0, SEEK_SET);
        if (!fread(buffer, 1, size, fp)) {
            free(buffer);
            fclose(fp);
            return MEPA_RC_ERROR;
        }
        jobj = json_tokener_parse(buffer);
        if (jobj == NULL) {
            free(buffer);
            fclose(fp);
            return MEPA_RC_ERROR;
        }
        if (!json_object_is_type(jobj, json_type_array)) {
            json_object_put(jobj);
            free(buffer);
            fclose(fp);
            return MEPA_RC_ERROR;
        }

        array_len = json_object_array_length(jobj);
        json_items = (json_object **)malloc(array_len * sizeof(json_object *));

        for (int i = 0; i < array_len; ++i) {
            json_items[i] = json_object_array_get_idx(jobj, i);
        }
        free(buffer);
    }
    fclose(fp);
    return MEPA_RC_OK;
}
// Function to write JSON to file
void write_json_to_file(const char *filename, json_object *jarray )
{

    FILE *file = fopen(filename, "w");

    if (file) {
        fprintf(file, "%s\n", json_object_to_json_string_ext(jarray, JSON_C_TO_STRING_PLAIN));
        fclose(file);

        // Free the JSON object
        json_object_put(jarray);
    } else {
        T_E("Error opening file");
    }
    T_I("Successfully stored PHY Data in %s", filename);
}

static void enable_warmstart(void)
{
    demo_phy_info_t phy_info;
    mepa_rc rc;
    for (int port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if ((rc = phy_family_detect(meba_phy_inst_wr, port_no, &phy_info)) == MEPA_RC_OK) {
            switch (phy_info.family) {
            case PHY_FAMILY_MALIBU_10G:
            case PHY_FAMILY_VIPER:
            case PHY_FAMILY_TESLA:
                // M10G, Viper, Tesla - ToDo
                break;

            case PHY_FAMILY_LAN8814:
                // Indy - Not supported
                break;

            case PHY_FAMILY_MALIBU_25G:
                rc = mepa_warmstart_conf_set(meba_phy_inst_wr->phy_devices[port_no], MEPA_RESTART_WARM);
                break;
            }
        }
    }
}

/*
Function: enable_ws_backup_phy_data
Enable Warm start and Backup PHY data to a JSON File
*/
static void backup_phy_data(void)
{
    json_rpc_req_t json_req = {};  // Initialize the JSON-RPC request structure
    json_object *jobj = NULL; // Pointer to hold the created JSON object
    struct json_object *jarray = json_object_new_array();
    json_req.ptr = json_req.buf;
    char phy_data_file[100];

    snprintf(phy_data_file, sizeof(phy_data_file), "/root/mepa_scripts/phy_data_ws.json");
    for (int port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if (gaphy_family[port_no] == PHY_FAMILY_MALIBU_25G) {
            phy25g_phy_state_t *phy25g_data_bkup = meba_phy_inst_wr->phy_devices[port_no]->data;

            if (json_rpc_new_phy25g_phy_state_t(&json_req, &jobj, phy25g_data_bkup) != MESA_RC_OK) {
                T_E("Error creating JSON Object from phy25g_phy_state_t\n");
            }
            json_object_array_add(jarray, jobj);
        }
    }
    write_json_to_file(phy_data_file, jarray);
}

static int mepa_drv_del()
{
    int base_ports[meba_phy_inst_wr->phy_device_cnt], n_base = 0;
    int port_no = 0;

    // Delete all M25G instance
    for (port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if (gaphy_family[port_no] == PHY_FAMILY_MALIBU_25G) {
            phy25g_phy_state_t *data = (phy25g_phy_state_t *)meba_phy_inst_wr->phy_devices[port_no]->data;
            mepa_device_t *base_dev = (mepa_device_t *)data->base_dev;
            phy25g_phy_state_t *base_data = (phy25g_phy_state_t *)base_dev->data;

            // Store Base port and delete at end
            if (base_data->port_no == port_no) {
                base_ports[n_base] = port_no;
                n_base++;
                continue;
            }
            T_I("Deleting dev port_no %u\n", port_no);
            if (meba_phy_inst_wr->phy_devices[port_no]) {
                if (mepa_delete(meba_phy_inst_wr->phy_devices[port_no]) != MEPA_RC_OK) {
                    T_E("Unable to delete the mepa device %d", port_no);
                    return MESA_RC_ERROR;
                }
                memset(&meba_phy_inst_wr->phy_device_ctx[port_no], 0, sizeof(mepa_callout_ctx_t));
                meba_phy_inst_wr->phy_devices[port_no] = NULL;
            } else {
                T_E("No Dev created for port_no %d\n", port_no);
                return MESA_RC_ERROR;
            }
        }
    }
    port_no = 0;
    for (int i = 0; i < n_base; i++) {
        port_no = base_ports[i];
        T_I("Deleting dev port_no %d\n", port_no);
        if (meba_phy_inst_wr->phy_devices[port_no]) {
            if (mepa_delete(meba_phy_inst_wr->phy_devices[port_no]) != MEPA_RC_OK) {
                T_E("Unable to delete the mepa device %d", port_no);
                return MESA_RC_ERROR;
            }
            memset(&meba_phy_inst_wr->phy_device_ctx[port_no], 0, sizeof(mepa_callout_ctx_t));
            meba_phy_inst_wr->phy_devices[port_no] = NULL;
        } else {
            T_E("No Dev created for port_no %d\n", port_no);
            return MESA_RC_ERROR;
        }
    }
    return MESA_RC_OK;
}

static int mepa_drv_create()
{
    meba_port_entry_t   entry;
    for (int port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if (gaphy_family[port_no] == PHY_FAMILY_MALIBU_25G) {
            T_I("Creating dev port no: %d\n", port_no);
            if (meba_phy_inst_wr->phy_devices[port_no] != NULL ) {
                T_E("Device Already existing on %d", port_no);
                continue; // Already probed
            }

            meba_phy_inst_wr->api.meba_port_entry_get(meba_phy_inst_wr, port_no, &entry);

            meba_phy_inst_wr->phy_device_ctx[port_no].inst = 0;
            meba_phy_inst_wr->phy_device_ctx[port_no].port_no = port_no;
            meba_phy_inst_wr->phy_device_ctx[port_no].meba_inst = meba_phy_inst_wr;
            meba_phy_inst_wr->phy_device_ctx[port_no].miim_controller = entry.map.miim_controller;
            meba_phy_inst_wr->phy_device_ctx[port_no].miim_addr = entry.map.miim_addr;
            meba_phy_inst_wr->phy_device_ctx[port_no].chip_no = entry.map.chip_no;
            board_conf.numeric_handle = port_no;

            meba_phy_inst_wr->phy_devices[port_no] = mepa_create(&(mepa_callout_wr),
                                                                 &meba_phy_inst_wr->phy_device_ctx[port_no],
                                                                 &board_conf);
        }
    }
    return MEPA_RC_OK;
}

static void cli_cmd_phy_warmstart_perform(cli_req_t *req)
{
    mepa_rc rc;
    meba_port_entry_t   entry;
    mepa_bool_t         warmstart_enabled = false, loadjson = false;
    mepa_restart_t      restart;
    mepa_device_t       *phy_dev;
    demo_phy_info_t     phy_info;

    cli_printf("Performing Warm Start\n");

    // Allocate memory for the global array
    gaphy_family = (int *)malloc((meba_phy_inst_wr->phy_device_cnt) * sizeof(int));

    // Check if warmstart is enabled if not, do not proceed
    for (int port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if ((rc = phy_family_detect(meba_phy_inst_wr, port_no, &phy_info)) == MEPA_RC_OK) {
            gaphy_family[port_no] = phy_info.family;

            switch (phy_info.family) {
            case PHY_FAMILY_MALIBU_10G:
            case PHY_FAMILY_VIPER:
            case PHY_FAMILY_TESLA:
                // M10G, Viper, Tesla - ToDo
                break;

            case PHY_FAMILY_LAN8814:
                // Indy - Not supported
                break;

            case PHY_FAMILY_MALIBU_25G:
                // Malibu 25G - Find if Warmstart is enabled
                rc = mepa_warmstart_conf_get(meba_phy_inst_wr->phy_devices[port_no], &restart);
                if ((MEPA_RC_OK == rc) && (MEPA_RESTART_WARM == restart)) {
                    T_I("Warm start enabled on %d", port_no);
                    warmstart_enabled = true;
                } else {
                    warmstart_enabled = false;
                }
                break;
            }
        }
    }
    if (warmstart_enabled == false) {
        cli_printf("Warm Start is disabled. Cannot proceed\n");
        // free memory
        free(gaphy_family);
        gaphy_family = NULL;
        return;
    }
    // Backup PHY Data
    backup_phy_data();

    // Kill PHY instance
    mepa_drv_del();

    // create PHY instance
    mepa_drv_create();

    // Enable accessing the shared resources by linking the base port on each port
    for (int port_no = 0; port_no < meba_phy_inst_wr->phy_device_cnt; port_no++) {
        if (gaphy_family[port_no] == PHY_FAMILY_MALIBU_25G) {
            meba_phy_inst_wr->api.meba_port_entry_get(meba_phy_inst_wr, port_no, &entry);
            phy_dev = meba_phy_inst_wr->phy_devices[port_no];
            if (phy_dev && meba_phy_inst_wr->phy_devices[entry.phy_base_port]) {
                (void)mepa_link_base_port(phy_dev,
                                          meba_phy_inst_wr->phy_devices[entry.phy_base_port],
                                          entry.map.chip_port);
            }

            if (warmstart_enabled) {
                if (!loadjson) {
                    if (load_phy_data() == 0) {
                        loadjson = true;
                    } else {
                        T_E("Failed to load PHY Data from JSON\n");
                    }
                }
                // Process JSON data and load data in struct
                if (loadjson) {
                    if (process_json_items(meba_phy_inst_wr->phy_devices[port_no], port_no) == 0) {
                        // Reinitialize base port
                        if (meba_phy_inst_wr->phy_devices[port_no] && meba_phy_inst_wr->phy_devices[entry.phy_base_port]) {
                            (void)mepa_link_base_port(meba_phy_inst_wr->phy_devices[port_no], meba_phy_inst_wr->phy_devices[entry.phy_base_port], entry.map.chip_port);
                        }
                    } else {
                        T_E("Failed to load PHY Data from JSON for port : %d\n", port_no);
                    }
                }
                rc = mepa_warmstart_conf_set(meba_phy_inst_wr->phy_devices[port_no], MEPA_RESTART_COLD);
            }

        }
    }
    // free memory
    free(gaphy_family);
    gaphy_family = NULL;
}

/* CLI Command Function to Enable PHY Warm start */
static void cli_cmd_phy_warmstart_enable(cli_req_t *req)
{
    // Function implementation goes here
    cli_printf("\n Enable Warm Start\n");

    enable_warmstart();

}

/* Initialize CLI Commands */
static void phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table) / sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
}

void mscc_appl_phy_restart(mscc_appl_init_t *init)
{
    meba_phy_inst_wr = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        phy_assign_callout();
        break;

    default:
        break;
    }

}
