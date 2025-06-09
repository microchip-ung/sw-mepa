// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <vtss_phy_api.h>
#include <microchip/ethernet/board/api.h>

#include "meba_aux.h"
#include "meba_generic.h"
#include <lan80xx_mcu.h>

/* Malibu25G PHY SKUs */
#define MEBA_25GPHY_8044  0x8044
#define MEBA_25GPHY_8043  0x8043
#define MEBA_25GPHY_8042  0x8042
#define MEBA_25GPHY_8023  0x8023
#define MEBA_25GPHY_8024  0x8024
#define MEBA_25GPHY_8022  0x8022
#define MEBA_10GPHY_8268  0x8268
#define MEBA_10GPHY_8267  0x8267
#define MEBA_10GPHY_8264  0x8264

/* Malibu 10G SKU */
#define MEBA_10GPHY_8258  0x8258


/* Malibu 10G SKU */
#define MEBA_10GPHY_8258  0x8258

#define HOST_INTR_B (6)
#define MEBA_GLOBAL_REG_DEV_ID      0x1E
#define MEBA_DEVICE_ID_REG_ADDR     0x0
#define MEBA_DEVICE_ID_SILICON_REV  0x2

typedef enum {
    PHY_TYPE_NONE,
    PHY10G_MAX_10G_SPEED,
    PHY25G_MAX_25G_SPEED,
    PHY25G_MAX_10G_SPEED,
} phy_type_t;

mepa_rc meba_mmd_read(struct mepa_callout_ctx           *ctx,
                      const uint8_t                      mmd,
                      const uint16_t                     addr,
                      uint16_t                          *const value)
{
    return mesa_mmd_read(ctx->inst, ctx->chip_no, ctx->miim_controller,
                         ctx->miim_addr, mmd, addr, value);
}

mepa_rc meba_mmd_read_inc(struct mepa_callout_ctx       *ctx,
                          const uint8_t                  mmd,
                          const uint16_t                 addr,
                          uint16_t                       *const buf,
                          uint8_t                        count)
{
    return mesa_port_mmd_read_inc(ctx->inst, ctx->port_no,
                                  mmd, addr, buf, count);
}

mepa_rc meba_mmd_write(struct mepa_callout_ctx          *ctx,
                       const uint8_t                     mmd,
                       const uint16_t                    addr,
                       const uint16_t                    value)
{
    return mesa_mmd_write(ctx->inst, ctx->chip_no, ctx->miim_controller,
                          ctx->miim_addr, mmd, addr, value);
}

mepa_rc meba_miim_read(struct mepa_callout_ctx          *ctx,
                       const uint8_t                     addr,
                       uint16_t                         *const value)
{
    return mesa_miim_read(ctx->inst, ctx->chip_no, ctx->miim_controller,
                          ctx->miim_addr, addr, value);
}

mepa_rc meba_miim_write(struct mepa_callout_ctx         *ctx,
                        const uint8_t                    addr,
                        const uint16_t                   value)
{
    return mesa_miim_write(ctx->inst, ctx->chip_no, ctx->miim_controller,
                           ctx->miim_addr, addr, value);
}

void *mem_alloc(struct mepa_callout_ctx *ctx, size_t size)
{
    return malloc(size);
}

void mem_free(struct mepa_callout_ctx *ctx, void *ptr)
{
    free(ptr);
}

uint8_t gpio_callback(void)
{
    uint8_t val;

    mesa_gpio_read(NULL, 0, HOST_INTR_B, &val);

    return val;
}

static void phy_detect_family(mepa_phy_info_t phy_info, phy_type_t *phy)
{
    switch (phy_info.part_number) {
    case MEBA_25GPHY_8044:
    case MEBA_25GPHY_8043:
    case MEBA_25GPHY_8042:
    case MEBA_25GPHY_8022:
    case MEBA_25GPHY_8023:
    case MEBA_25GPHY_8024:
        *phy = PHY25G_MAX_25G_SPEED;
        break;
    case MEBA_10GPHY_8268:
    case MEBA_10GPHY_8267:
    case MEBA_10GPHY_8264:
        *phy = PHY25G_MAX_10G_SPEED;
        break;
    case MEBA_10GPHY_8258:
        *phy = PHY10G_MAX_10G_SPEED;
        break;
    default:
        *phy = PHY_TYPE_NONE;
        break;
    }
    return;
}

static void meba_phy_config(meba_inst_t inst, const vtss_inst_t vtss_instance, mepa_port_no_t port_no, mepa_port_no_t base_port_no)
{
    mepa_phy_info_t phy_info = {};
    mepa_reset_param_t phy_reset = {};
    phy_type_t   phy_type = PHY_TYPE_NONE;
    /* Pre Reset Configuration */
    phy_reset.media_intf = MESA_PHY_MEDIA_IF_FI_10G_LAN;
    phy_reset.reset_point = MEPA_RESET_POINT_PRE;
    meba_phy_reset(inst, port_no, &phy_reset);

    meba_phy_info_get(inst, port_no, &phy_info);

    phy_detect_family(phy_info, &phy_type);

    if (((phy_type == PHY25G_MAX_10G_SPEED) || (phy_type == PHY25G_MAX_25G_SPEED)) && (port_no == base_port_no)) {
#ifdef MEPA_DEMO_EDSx
        lan80xx_MB_INTR_register_callback(inst->phy_devices[port_no], gpio_callback);
#endif
    }

    /* Default Reset Point */
    phy_reset.reset_point = MEPA_RESET_POINT_DEFAULT;
    meba_phy_reset(inst, port_no, &phy_reset);

    /* Post Reset Point */
    phy_reset.reset_point = MEPA_RESET_POINT_POST;
    meba_phy_reset(inst, port_no, &phy_reset);

    if (phy_info.part_number == MEBA_10GPHY_8258) {
        m10g_mode_conf(vtss_instance, inst, port_no, base_port_no);
    }
    if (phy_type == PHY25G_MAX_25G_SPEED) {
        /* LAN80XX PHYs Port Config to 25G Speed */
        lan80xx_phy_conf(inst, port_no, base_port_no, MEBA_PHY_PORT_MAX_25G_SPEED);
    } else if (phy_type == PHY25G_MAX_10G_SPEED) {
        /* LAN80XX PHYs Port Config to 10G Speed */
        lan80xx_phy_conf(inst, port_no, base_port_no, MEBA_PHY_PORT_MAX_10G_SPEED);
    }
}

/* MDIO PHY Address of each ports */
uint16_t slot1_map[] = {0x1b, 0x1a, 0x19, 0x18};
//uint16_t slot1_map[] = {0x3, 0x2, 0x1, 0x0};  /* PHY address of Malibu25G ASIC */
uint16_t slot1_map_viper[] = {0x0, 0x1, 0x2, 0x3};
uint16_t slot2_map[] = {0x1f, 0x1e, 0x1d, 0x1c};

/*PHY addr 24,25,26 & 27 in 25G slot 1 (port no 12,13,14,15)
  12 --> 27
  13 --> 26
  14 --> 25
  15 --> 24
  This slot has mii bus (SPI and mdio),
  first perform read in this slot for any of the address with mdio
  or perform spi read ch (0,1,2,3) with spidev0.1 in this slot
  identify the PHY and configure the default configs.
  if eeprom is available read via i2c-3 and configure based on the hardware
  details */
void phy_25g_slot1_scan(meba_inst_t inst, mepa_port_no_t port_no, meba_port_entry_t *entry, mepa_port_no_t base_port)
{
    /* seperate API support needed for slot 2 SPI as of now only spi1 taken */
    uint32_t phy_id = 0, rev = 0;
    uint32_t model = 0;
    uint16_t reg2 = 0, reg3 = 0;
    uint8_t data = 0;
    mepa_bool_t  lan80xx_phy = 1;

    if (inst->iface.mepa_spi_slot1_reg_read != NULL) {
        inst->iface.mepa_spi_slot1_reg_read(NULL, port_no, MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_REG_ADDR, &phy_id);
        inst->iface.mepa_spi_slot1_reg_read(NULL, port_no, MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_SILICON_REV, &rev);
    } else {
        mesa_mmd_read(0, entry->map.chip_no, 0, slot1_map[port_no % 4], MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_REG_ADDR, (uint16_t *)&phy_id);
        mesa_mmd_read(0, entry->map.chip_no, 0, slot1_map[port_no % 4], MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_SILICON_REV, (uint16_t *)&rev);
        entry->map.miim_addr       =  slot1_map[port_no % 4];
    }
    /* SW Work-arround for A0 revision of LAN80XX PHYs */
    if ((phy_id == 0) && (rev == 0xA0)) {
        phy_id = MEBA_25GPHY_8044;
    }
    if (phy_id == MEBA_25GPHY_8044 || phy_id == MEBA_25GPHY_8043 || phy_id == MEBA_25GPHY_8042) {
        /* 25G Capable Quad-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_25G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_25G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = base_port;

    } else if (phy_id == MEBA_10GPHY_8258 || phy_id == MEBA_10GPHY_8267 || phy_id == MEBA_10GPHY_8268) {
        /* 10G Capable Quad-PHYs*/
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_VTSS_10G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = base_port;
        lan80xx_phy  = (phy_id == MEBA_10GPHY_8258) ? 0 : 1;

    } else if ((phy_id == MEBA_25GPHY_8024 || phy_id == MEBA_25GPHY_8023 || phy_id == MEBA_25GPHY_8022)) {
        /* 25G Capable Dual-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_25G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_25G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = (base_port + 2);
    } else if (phy_id == MEBA_10GPHY_8264) {
        /* 10G Capable Dual-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_VTSS_10G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = (base_port + 2);
    } else {
        lan80xx_phy = 0;
        mesa_miim_read(0, entry->map.chip_no, 0, slot1_map_viper[port_no % 4], 2, &reg2);
        mesa_miim_read(0, entry->map.chip_no, 0, slot1_map_viper[port_no % 4], 3, &reg3);
        phy_id = (((uint32_t)reg2) << 16) | reg3;
        model = ((phy_id & 0xffff0) >> 4);
        if ((model == 0x707c) || (model == 0x707b) || (model == 0x707d)) {
            entry->mac_if              = MESA_PORT_INTERFACE_QSGMII;
            entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
            entry->cap = (MEBA_PORT_CAP_1G_PHY | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_SFP_DETECT | MEBA_PORT_CAP_SFP_1G |
                          MEBA_PORT_CAP_DUAL_FIBER_1000X | MEBA_PORT_CAP_10M_HDX | MEBA_PORT_CAP_10M_FDX |
                          MEBA_PORT_CAP_100M_HDX | MEBA_PORT_CAP_100M_FDX );
            entry->map.miim_addr       =  slot1_map_viper[port_no % 4];
            entry->phy_base_port       = base_port;
        }
    }

    /* Work-arround for LAN80XX EVB Board
     * MCP9902 I2C Slave device in EVB board is always generating "ALERT" signal due to fault condition detected by thremal diode of Malibu25G
     * As "ALERT" signal is high, it is holding the SFP_IRQ line
     * To demostrate LAN80XX Agregate interrupt, the ALERT signal from MCP9902 device is masked by writing address 0x3 with value 0x80
     * I2C slave address of MCP9902 is 0x4C
     */
    if(lan80xx_phy) {
        data = 0x80;
        inst->api.meba_sfp_i2c_xfer(inst, port_no, TRUE, 0x4C, 0x3, &data, 1, FALSE);
    }

    T_I(inst, "port(%d):  %x\n", port_no, phy_id & 0xFFFF);

}

void phy_25g_slot2_scan(meba_inst_t inst, mepa_port_no_t port_no, meba_port_entry_t *entry, mepa_port_no_t base_port)
{
    uint32_t phy_id = 0, rev = 0;
    uint8_t data = 0;
    mepa_bool_t  lan80xx_phy = 1;

    if (inst->iface.mepa_spi_slot2_reg_read != NULL) {
        inst->iface.mepa_spi_slot2_reg_read(NULL, port_no, MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_REG_ADDR, &phy_id);
        inst->iface.mepa_spi_slot2_reg_read(NULL, port_no, MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_SILICON_REV, &rev);
    } else {
        mesa_mmd_read(0, entry->map.chip_no, 0, slot2_map[port_no % 4], MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_REG_ADDR, (uint16_t *)&phy_id);
        mesa_mmd_read(0, entry->map.chip_no, 0, slot2_map[port_no % 4], MEBA_GLOBAL_REG_DEV_ID, MEBA_DEVICE_ID_SILICON_REV, (uint16_t *)&rev);
        entry->map.miim_addr       =  slot2_map[port_no % 4];
    }
    /* SW Work-arround for A0 revision of LAN80XX PHYs */
    if ((phy_id == 0) && (rev == 0xA0)) {
        phy_id = MEBA_25GPHY_8044;
    }
    if (phy_id == MEBA_25GPHY_8044 || phy_id == MEBA_25GPHY_8043 || phy_id == MEBA_25GPHY_8042) {
        /* 25G Capable Quad-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_25G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_25G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = base_port;

    } else if (phy_id == MEBA_10GPHY_8258 || phy_id == MEBA_10GPHY_8267 || phy_id == MEBA_10GPHY_8268) {
        /* 10G Capable Quad-PHYs*/
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_VTSS_10G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = base_port;
        lan80xx_phy  = (phy_id == MEBA_10GPHY_8258) ? 0 : 1;

    } else if ((phy_id == MEBA_25GPHY_8024 || phy_id == MEBA_25GPHY_8023 || phy_id == MEBA_25GPHY_8022)) {
        /* 25G Capable Dual-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_25G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_25G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = (base_port + 2);
    } else if (phy_id == MEBA_10GPHY_8264) {
        /* 10G Capable Dual-PHYs */
        entry->mac_if              = MESA_PORT_INTERFACE_SFI;
        entry->map.miim_controller = MESA_MIIM_CONTROLLER_0;
        entry->cap = (MEBA_PORT_CAP_VTSS_10G_PHY | MEBA_PORT_CAP_10G_FDX | MEBA_PORT_CAP_FLOW_CTRL | MEBA_PORT_CAP_1G_FDX | MEBA_PORT_CAP_AUTONEG |
                      MEBA_PORT_CAP_SFP_DETECT);
        entry->phy_base_port       = (base_port + 2);
    }

    /* Work-arround for LAN80XX EVB Board
     * MCP9902 I2C Slave device in EVB board is always generating "ALERT" signal due to fault condition detected by thremal diode of Malibu25G
     * As "ALERT" signal is high, it is holding the SFP_IRQ line
     * To demostrate LAN80XX Agregate interrupt, the ALERT signal from MCP9902 device is masked by writing address 0x3 with value 0x80
     * I2C slave address of MCP9902 is 0x4C
     */
    if(lan80xx_phy) {
        data = 0x80;
        inst->api.meba_sfp_i2c_xfer(inst, port_no, TRUE, 0x4C, 0x3, &data, 1, FALSE);
    }
    T_I(inst, "port(%d):  %x\n", port_no, phy_id & 0xFFFF);

}



// The meba_port_entry_get returns the MAC if-type
// The RGMII Internal Delay cannot be identical between MAC and Phy
static mepa_port_interface_t rgmii_id_convert(mepa_port_interface_t interface)
{
    switch (interface) {
    case MESA_PORT_INTERFACE_RGMII:
        return MESA_PORT_INTERFACE_RGMII_ID;
    case MESA_PORT_INTERFACE_RGMII_ID:
        return MESA_PORT_INTERFACE_RGMII;
    case MESA_PORT_INTERFACE_RGMII_RXID:
        return MESA_PORT_INTERFACE_RGMII_TXID;
    case MESA_PORT_INTERFACE_RGMII_TXID:
        return MESA_PORT_INTERFACE_RGMII_RXID;
    default:
        return interface;
    }
}


void meba_phy_driver_init(meba_inst_t inst)
{
    mepa_rc             rc;
    mesa_port_no_t      port_no;
    meba_port_entry_t   entry;
    mepa_device_t       *phy_dev;
    mepa_port_no_t      base_port_no = 0;

    inst->phy_device_ctx = calloc(inst->phy_device_cnt, sizeof(mepa_callout_ctx_t));
    inst->spi_mepa_callout = calloc(inst->phy_device_cnt, sizeof(mepa_callout_t));
    inst->mepa_callout.mmd_read = meba_mmd_read;
    inst->mepa_callout.mmd_read_inc = meba_mmd_read_inc;
    inst->mepa_callout.mmd_write = meba_mmd_write;
    inst->mepa_callout.miim_read = meba_miim_read;
    inst->mepa_callout.miim_write = meba_miim_write;
    inst->mepa_callout.spi_read = inst->iface.mepa_spi_slot1_reg_read;
    inst->mepa_callout.spi_write = inst->iface.mepa_spi_slot1_reg_write;
    inst->mepa_callout.lock_enter = inst->iface.lock_enter;
    inst->mepa_callout.lock_exit = inst->iface.lock_exit;
    inst->mepa_callout.mem_alloc = mem_alloc;
    inst->mepa_callout.mem_free = mem_free;
    MEPA_TRACE_FUNCTION = inst->iface.trace;
    memset(&entry, 0, sizeof(meba_port_entry_t));
    vtss_inst_t vtss_instance = NULL;

    for (port_no = 0; port_no < inst->phy_device_cnt; port_no++) {
        mepa_board_conf_t board_conf = {};
        board_conf.numeric_handle = port_no;
        // Identify and initialize PHYs (no CuSFPs at this point)
        // This PHY initialization needs to take place before the
        // port interface is initialized.
        // Reason for that is that some PHYs perform calibration
        // steps during their initialization that require no
        // activity on the MAC interface as it interferes with
        // the calibration.
        if (inst->phy_devices[port_no] != NULL ) {
            continue; // Already probed
        }
        inst->api.meba_port_entry_get(inst, port_no, &entry);
        mepa_port_interface_t mac_if = rgmii_id_convert(entry.mac_if);
        meba_port_cap_t port_cap = entry.cap;

        if ((port_cap & (MEBA_PORT_CAP_COPPER | MEBA_PORT_CAP_DUAL_COPPER)) || (port_cap & MEBA_PORT_CAP_VTSS_10G_PHY) || (port_cap & MEBA_PORT_CAP_25G_PHY)) {
            inst->phy_device_ctx[port_no].inst = 0;
            inst->phy_device_ctx[port_no].port_no = port_no;
            inst->phy_device_ctx[port_no].meba_inst = inst;
            inst->phy_device_ctx[port_no].miim_controller = entry.map.miim_controller;
            inst->phy_device_ctx[port_no].miim_addr = entry.map.miim_addr;
            inst->phy_device_ctx[port_no].chip_no = entry.map.chip_no;
            if (!inst->iface.phy_attach) {
                continue;
            }
            inst->phy_devices[port_no] = mepa_create(&(inst->mepa_callout),
                                                     &(inst->phy_device_ctx[port_no]),
                                                     &board_conf);
            if (inst->phy_devices[port_no]) {
                T_I(inst, "Phy has been probed on port %d, MAC I/F = %d", port_no, mac_if);
                rc = mepa_if_set(inst->phy_devices[port_no], mac_if);
                if (rc != MESA_RC_OK && rc != MESA_RC_NOT_IMPLEMENTED) {
                    T_E(inst, "Failed to set MAC interface on PHY: %d (MAC I/F = %d), rc = %d = 0x%x", port_no, mac_if, rc, rc);
                }
                vtss_instance = board_conf.vtss_instance_ptr;
            } else {
                T_I(inst, "Probe failed on %d", port_no);
            }
        }
    }

    // Enable accessing the shared resources by linking the base port on each port
    for (port_no = 0; port_no < inst->phy_device_cnt; port_no++) {
        inst->api.meba_port_entry_get(inst, port_no, &entry);
        phy_dev = inst->phy_devices[port_no];
        if (phy_dev && inst->phy_devices[entry.phy_base_port]) {
            (void)mepa_link_base_port(phy_dev,
                                      inst->phy_devices[entry.phy_base_port],
                                      entry.map.chip_port);

            base_port_no = entry.phy_base_port;
        }
        meba_phy_config(inst, vtss_instance, port_no, base_port_no);
    }

}
