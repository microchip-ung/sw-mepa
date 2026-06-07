// Copyright (c) 2026 Vincent Jardin, Vincent Cruz, Free Mobile
// SPDX-License-Identifier: MIT

// PHY-only mode implementation: see phy_only.h for the rationale.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "trace.h"
#include "phy_only.h"
#include "spi_proxy/spiproxy.h"

// by default, keep legacy mode
mesa_bool_t phy_only_mode = 0;

void phy_only_enter(void)
{
    phy_only_mode = 1;
    printf("No switch UIO device found: PHY-only mode (MEPA slots over SPI only)\n");
}

mesa_rc phy_only_reg_read(const mesa_chip_no_t chip_no,
                          const uint32_t addr,
                          uint32_t *const value)
{
    (void)chip_no;
    (void)addr;
    (void)value;
    return MESA_RC_ERROR; /* no switch on this board */
}

mesa_rc phy_only_reg_write(const mesa_chip_no_t chip_no,
                           const uint32_t addr,
                           const uint32_t value)
{
    (void)chip_no;
    (void)addr;
    (void)value;
    return MESA_RC_ERROR; /* no switch on this board */
}

// Serve the minimal answer the MEBA layer needs and fail
// everything else so callers fall back to their defaults:
// "pcb" = 134 selects the PCB134 board type, whose EDSx path hardcodes
// port_cnt = 9 (PHY slot 1 = ports 0..3) and does not touch switch registers during
// meba_initialize().
mesa_rc phy_only_board_conf_get(const char *tag, char *buf,
                                size_t bufsize, size_t *buflen)
{
    if (strcmp(tag, "pcb") == 0) {
        size_t len = snprintf(buf, bufsize, "134");
        if (buflen) {
            *buflen = len;
        }
        return MESA_RC_OK;
    }
    return MESA_RC_ERROR;
}

// The init_modules() subset that is safe without a MESA switch
// instance: the skipped modules (port, MAC table, VLAN, FDMA packet IO,
// IP, symreg, test, KR, example, UIO IRQs) all drive switch state that
// does not exist here. main_init() and the CLI module have already run
// when this is called.
void phy_only_init_modules(mscc_appl_init_t *init)
{
    if (init->cmd == MSCC_INIT_CMD_INIT) {
        // Normally issued by the (skipped) port module: wires
        // inst->phy_devices to the board state array and installs the
        // MEPA callouts; without it every mepa_dev_create_check() in
        // the demo modules dereferences a NULL phy_devices pointer on
        // the first poll pass.
        MEBA_WRAP(meba_reset, init->board_inst, MEBA_PHY_INITIALIZE);
    }
    mscc_appl_json_rpc_init(init);
    mscc_appl_debug_init(init);
    mscc_appl_trace_init(init);
    mscc_appl_spi_init(init);
    mscc_appl_phy_init(init);
    mscc_appl_kat_demo(init);
    mscc_appl_phy_synce(init);
    mepa_demo_appl_macsec_demo(init);
    mepa_demo_appl_gpio_lp_demo(init);
    mscc_appl_phy_loopback_init(init);
    mscc_appl_phy_xconnect(init);
    mscc_appl_phy_diagnostics_demo(init);
    mscc_appl_phy_restart(init);
    mscc_appl_phy_kr_init(init);
    mepa_demo_appl_ts_demo(init);
    mepa_demo_appl_macsec_rollover_demo(init);
#ifdef MEPA_HAS_LAN80XX
    mscc_appl_m25gdiag_demo(init);
    mscc_appl_mcu_fw_init(init);
#endif
}

//
// Configurable PHY slots (-P option), see phy_only.h.
//
#define PHY_ONLY_SLOT_MAX    8
#define PHY_ONLY_SPI_BYTES   7  /* 3 address + 4 data, as in spi.c */
#define PHY_ONLY_SPI_PAD_MAX 15

#define PHY_ONLY_DEVICE_ID_MMD 0x1E
#define PHY_ONLY_DEVICE_ID_REG 0x0

typedef struct {
    char     dev[sizeof(((struct sockaddr_un *)0)->sun_path)];
    int      pad;
    int      freq;
    uint32_t ports;      /* demo ports covered by this slot */
    uint32_t base;       /* first demo port of this slot */
    int      fd;
    int      proxy;      /* 0 = direct spidev, 1 = via lan80xx-spid */
    uint32_t seq;        /* proxy request sequence */
} phy_only_slot_t;

static phy_only_slot_t phy_only_slot[PHY_ONLY_SLOT_MAX];
static int phy_only_slot_cnt;

// Optional SPI op trace (pre-analysis/debugging)
// set env PHY_ONLY_SPI_TRACE=<file> to log one line per
// logical register access: "<monotonic-us> <port> <R|W> <mmd> <reg>".
// It is Buffered so tracing does not dominate the per-op cost; reads cost
// two SPI transfers on the wire (pipelined dummy read).
static FILE *phy_only_trace_fp = NULL;

static void phy_only_trace_init(void)
{
    const char *path = getenv("PHY_ONLY_SPI_TRACE");

    if (path != NULL && (phy_only_trace_fp = fopen(path, "w")) != NULL) {
        setvbuf(phy_only_trace_fp, NULL, _IOLBF, 0);
        printf("PHY slot SPI trace -> %s\n", path);
    }
}

static void phy_only_trace_op(mepa_port_no_t port_no, mesa_bool_t read,
                              uint8_t mmd, uint16_t reg_num)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    fprintf(phy_only_trace_fp, "%llu %u %c %02x %04x\n",
            (unsigned long long)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000,
            port_no, read ? 'R' : 'W', mmd, reg_num);
}

int phy_only_slots_configured(void)
{
    return phy_only_slot_cnt;
}

static mesa_rc phy_only_slot_opt(char *parm)
{
    phy_only_slot_t *slot;
    char *s;

    if (phy_only_slot_cnt >= PHY_ONLY_SLOT_MAX) {
        fprintf(stderr, "-P: at most %d PHY slots\n", PHY_ONLY_SLOT_MAX);
        return MESA_RC_ERROR;
    }
    slot = &phy_only_slot[phy_only_slot_cnt];
    slot->pad = 1;
    slot->freq = 15000000;
    slot->ports = 4;

    if (strncmp(parm, "proxy:", 6) == 0) {
        // via lan80xx-spid: proxy:<socket-path>[:ports], pad/freq
        // belong to the daemon. ':ports' = trailing ':' + digits.
        slot->proxy = 1;
        parm += 6;
        if ((s = strrchr(parm, ':')) != NULL && s[1] >= '0' && s[1] <= '9') {
            *s++ = 0;
            slot->ports = atoi(s);
        }
    } else {
        if ((s = strchr(parm, ':')) != NULL) {
            *s++ = 0;
            slot->ports = atoi(s);
        }
        if ((s = strchr(parm, '@')) != NULL) {
            *s++ = 0;
            slot->pad = atoi(s);
            if (slot->pad > PHY_ONLY_SPI_PAD_MAX) {
                slot->pad = PHY_ONLY_SPI_PAD_MAX;
            }
            if ((s = strchr(s, '@')) != NULL) {
                *s++ = 0;
                slot->freq = atoi(s);
            }
        }
    }
    if (slot->ports < 1 || slot->ports > 4) {
        fprintf(stderr, "-P: ports must be 1..4\n");
        return MESA_RC_ERROR;
    }
    if ((size_t)snprintf(slot->dev, sizeof(slot->dev), "%s", parm) >= sizeof(slot->dev)) {
        fprintf(stderr, "-P: path too long (max %zu chars)\n", sizeof(slot->dev) - 1);
        return MESA_RC_ERROR;
    }
    slot->base = phy_only_slot_cnt ?
                 (phy_only_slot[phy_only_slot_cnt - 1].base +
                  phy_only_slot[phy_only_slot_cnt - 1].ports) : 0;
    phy_only_slot_cnt++;
    if (slot->proxy) {
        printf("PHY slot %d: proxy %s, ports %u..%u\n",
               phy_only_slot_cnt, slot->dev, slot->base,
               slot->base + slot->ports - 1);
    } else {
        printf("PHY slot %d: %s, ports %u..%u, %d padding byte(s) at %d Hz\n",
               phy_only_slot_cnt, slot->dev, slot->base,
               slot->base + slot->ports - 1, slot->pad, slot->freq);
    }
    return MESA_RC_OK;
}

static mscc_appl_opt_t phy_only_opt = {
    "P:",
    "<spidev[@pad[@freq]]|proxy:sock>[:ports]",
    "PHY-only slot: one PHY package serving 'ports' (default 4 =\n"
    "                          quad footprint; repeatable; forces PHY-only mode and replaces\n"
    "                          the built-in slot devices). Direct spidev access, or shared\n"
    "                          access through the SPI proxy socket (proxy:<path>).",
    phy_only_slot_opt
};

void phy_only_opt_reg(void)
{
    mscc_appl_opt_reg(&phy_only_opt);
}

// Connect (or reconnect after a daemon restart) a proxy-mode slot
static mesa_rc phy_only_proxy_connect(phy_only_slot_t *slot)
{
    struct sockaddr_un sa = { .sun_family = AF_UNIX };
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    snprintf(sa.sun_path, sizeof(sa.sun_path), "%s", slot->dev);
    if (fd < 0 || connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        fprintf(stderr, "proxy %s: %s\n", slot->dev, strerror(errno));
        if (fd >= 0) {
            close(fd);
        }
        slot->fd = -1;
        return MESA_RC_ERROR;
    }
    slot->fd = fd;
    return MESA_RC_OK;
}

mesa_rc phy_only_slots_open(void)
{
    int i, fd, mode = 0;

    phy_only_trace_init();
    for (i = 0; i < phy_only_slot_cnt; i++) {
        if (phy_only_slot[i].proxy) {
            if (phy_only_proxy_connect(&phy_only_slot[i]) != MESA_RC_OK) {
                return MESA_RC_ERROR;
            }
            continue;
        }
        fd = open(phy_only_slot[i].dev, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "%s: %s\n", phy_only_slot[i].dev, strerror(errno));
            return MESA_RC_ERROR;
        }
        if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 ||
            ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) {
            fprintf(stderr, "%s: cannot set spi mode\n", phy_only_slot[i].dev);
            close(fd);
            return MESA_RC_ERROR;
        }
        phy_only_slot[i].fd = fd;
    }
    return MESA_RC_OK;
}

// One SPI transfer in the spi.c frame format: 23-bit address
// (bit 23 = write) + 32-bit data; reads append the slot's padding bytes.
static mesa_rc phy_only_xfer(phy_only_slot_t *slot, mesa_bool_t read,
                             uint32_t addr, uint32_t *const data)
{
    uint8_t tx[PHY_ONLY_SPI_BYTES + PHY_ONLY_SPI_PAD_MAX];
    uint8_t rx[sizeof(tx)] = { 0 };
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = PHY_ONLY_SPI_BYTES + (read ? slot->pad : 0),
        .speed_hz = slot->freq,
        .bits_per_word = 8,
    };

    memset(tx, 0xff, sizeof(tx));
    tx[0] = (uint8_t)((read ? 0 : 0x80) | ((addr >> 16) & 0x7f));
    tx[1] = (uint8_t)(addr >> 8);
    tx[2] = (uint8_t)(addr >> 0);
    if (!read) {
        tx[3] = (uint8_t)(*data >> 24);
        tx[4] = (uint8_t)(*data >> 16);
        tx[5] = (uint8_t)(*data >> 8);
        tx[6] = (uint8_t)(*data >> 0);
    }
    if (ioctl(slot->fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        return MESA_RC_ERROR;
    }
    if (read) {
        *data = (rx[3] << 24) | (rx[4] << 16) | (rx[5] << 8) | rx[6];
    }
    return MESA_RC_OK;
}

// Register access through the SPI proxy:
// one L1 READ/WRITE request/response (the daemon handles the pipelined
// dummy read).
// One transparent reconnect attempt for a daemon restart
static mesa_rc phy_only_proxy_rw(phy_only_slot_t *slot, uint32_t ch_no,
                                 mesa_bool_t read, uint8_t mmd,
                                 uint16_t reg_num, uint32_t *const data)
{
    struct {
        struct spiproxy_hdr h;
        struct spiproxy_op op;
    } msg;
    int retry;
    ssize_t n;

    for (retry = 0; retry < 2; retry++) {
        if (slot->fd < 0 && phy_only_proxy_connect(slot) != MESA_RC_OK) {
            return MESA_RC_ERROR;
        }
        memset(&msg, 0, sizeof(msg));
        msg.h.ver = SPIPROXY_VER;
        msg.h.type = read ? SPIPROXY_READ : SPIPROXY_WRITE;
        msg.h.seq = ++slot->seq;
        msg.h.len = sizeof(msg.op);
        msg.op.slice = (uint8_t)ch_no;
        msg.op.write = !read;
        msg.op.mmd = mmd;
        msg.op.reg = reg_num;
        msg.op.val = read ? 0 : *data;
        if (send(slot->fd, &msg, sizeof(msg), MSG_NOSIGNAL) < 0 ||
            (n = recv(slot->fd, &msg, sizeof(msg), 0)) < (ssize_t)sizeof(msg.h)) {
            close(slot->fd);
            slot->fd = -1;
            continue; /* daemon gone: reconnect once */
        }
        if (msg.h.seq != slot->seq ||
            msg.h.type != ((read ? SPIPROXY_READ : SPIPROXY_WRITE) | SPIPROXY_RESP) ||
            msg.h.flags != SPIPROXY_OK) {
            return MESA_RC_ERROR;
        }
        if (read) {
            *data = msg.op.val;
        }
        return MESA_RC_OK;
    }
    return MESA_RC_ERROR;
}

// SPI register access for a demo port through the -P slot table. Same
// register/channel encoding as the built-in slots.
mesa_rc phy_only_spi_rw(mepa_port_no_t port_no, mesa_bool_t read,
                        uint8_t mmd, uint16_t reg_num, uint32_t *const data)
{
    phy_only_slot_t *slot = NULL;
    uint32_t ch_no, addr;
    int i;

    for (i = 0; i < phy_only_slot_cnt; i++) {
        if (port_no >= phy_only_slot[i].base &&
            port_no < phy_only_slot[i].base + phy_only_slot[i].ports) {
            slot = &phy_only_slot[i];
            break;
        }
    }
    if (slot == NULL) {
        return MESA_RC_ERROR;
    }
    if (phy_only_trace_fp != NULL) {
        phy_only_trace_op(port_no, read, mmd, reg_num);
    }
    ch_no = slot->base + slot->ports - 1 - port_no;
    if (slot->proxy) {
        return phy_only_proxy_rw(slot, ch_no, read, mmd, reg_num, data);
    }
    if (slot->fd <= 0) {
        return MESA_RC_ERROR;
    }
    addr = ch_no << 21 | mmd << 16 | reg_num;
    if (read) {
        if (phy_only_xfer(slot, 1, addr, data) != MESA_RC_OK) {
            return MESA_RC_ERROR;
        }
        addr = ch_no << 21 | PHY_ONLY_DEVICE_ID_MMD << 16 | PHY_ONLY_DEVICE_ID_REG;
        return phy_only_xfer(slot, 1, addr, data);
    }
    return phy_only_xfer(slot, 0, addr, data);
}
