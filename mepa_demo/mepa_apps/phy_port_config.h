// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

//port config application header
mepa_rc meba_miim_read(struct mepa_callout_ctx          *ctx,
                       const uint8_t                     addr,
                       uint16_t                         *const value);

mepa_rc meba_miim_write(struct mepa_callout_ctx         *ctx,
                        const uint8_t                    addr,
                        const uint16_t                   value);

mepa_rc meba_mmd_read(struct mepa_callout_ctx           *ctx,
                      const uint8_t                      mmd,
                      const uint16_t                     addr,
                      uint16_t                          *const value);

mepa_rc meba_mmd_read_inc(struct mepa_callout_ctx       *ctx,
                          const uint8_t                  mmd,
                          const uint16_t                 addr,
                          uint16_t                       *const buf,
                          uint8_t                        count);

mepa_rc meba_mmd_write(struct mepa_callout_ctx          *ctx,
                       const uint8_t                     mmd,
                       const uint16_t                    addr,
                       const uint16_t                    value);


mesa_rc mepa_spi_reg_read_write (void *chip,
                                 mepa_port_no_t port_no,
                                 mepa_bool_t           read,
                                 uint8_t             dev,
                                 uint16_t            reg_num,
                                 uint32_t            *const data);

mesa_rc mepa_spi2_reg_read_write (void *chip,
                                  mepa_port_no_t port_no,
                                  mepa_bool_t           read,
                                  uint8_t             dev,
                                  uint16_t            reg_num,
                                  uint32_t            *const data);

void *mem_alloc(struct mepa_callout_ctx *ctx, size_t size);

void mem_free(struct mepa_callout_ctx *ctx, void *ptr);


mesa_rc mepa_phy_spi_read (struct mepa_callout_ctx *ctx,
                           mepa_port_no_t port_no,
                           uint8_t             dev,
                           uint16_t            reg_num,
                           uint32_t            *const data);


mesa_rc mepa_phy_spi_write (struct mepa_callout_ctx *ctx,
                            mepa_port_no_t port_no,
                            uint8_t             dev,
                            uint16_t            reg_num,
                            uint32_t            *const data);

mesa_rc mepa_spi2_spi_write (struct mepa_callout_ctx *ctx,
                             mepa_port_no_t port_no,
                             uint8_t             dev,
                             uint16_t            reg_num,
                             uint32_t            *const data);
