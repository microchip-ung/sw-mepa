//-----------------------------------------------------------------------------
// LSD Generator
//-----------------------------------------------------------------------------
// Perl Package        : LSD::generator::targetC (v1.1)
// LSD Source          : /home/p31g/p31g_chip/v_emouchel.priv.p31g_chip.registers/ipg_lsd/lsd_sys/source_32b/xml/reg_files/EIPE_EIP160_EIP62_EIP62_Registers_def.xml
// Register File Name  : EIPE_EIP160_EIP62_EIP62_REGISTERS
// Register File Title : EIPE EIP160_EIP62_EIP62_Registers
// Register Width      : 32
// Note                : Doxygen compliant comments
//-----------------------------------------------------------------------------

#ifndef _EIPE_EIP160_EIP62_EIP62_REGISTERS_H
#define _EIPE_EIP160_EIP62_EIP62_REGISTERS_H

//! \defgroup EIPE_EIP160_EIP62_EIP62_REGISTERS Register File EIPE_EIP160_EIP62_EIP62_REGISTERS - EIPE EIP160_EIP62_EIP62_Registers
//! @{

//! Base Address of EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_MODULE_BASE 0x00B2F400u

//! \defgroup EIPE_EIP62_TOKEN_CTRL_STAT Register EIPE_EIP62_TOKEN_CTRL_STAT - EIPE EIP62 TOKEN CTRL STAT
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_TOKEN_CTRL_STAT 0x0
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_TOKEN_CTRL_STAT 0x00B2F400u

//! Register Reset Value
#define EIPE_EIP62_TOKEN_CTRL_STAT_RST 0x00000004u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_RESERVED_0_POS 0
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_RESERVED_0_MASK 0x3u

//! Field TOKEN_LOCATION_AVAILABLE - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_TOKEN_LOCATION_AVAILABLE_POS 2
//! Field TOKEN_LOCATION_AVAILABLE - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_TOKEN_LOCATION_AVAILABLE_MASK 0x4u

//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_RESERVED_1_POS 3
//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_TOKEN_CTRL_STAT_RESERVED_1_MASK 0xFFFFFFF8u

//! @}

//! \defgroup EIPE_EIP62_PROT_ALG_EN Register EIPE_EIP62_PROT_ALG_EN - EIPE EIP62 PROT ALG EN
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_PROT_ALG_EN 0x4
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_PROT_ALG_EN 0x00B2F404u

//! Register Reset Value
#define EIPE_EIP62_PROT_ALG_EN_RST 0x800004D9u

//! Field HASH_ONLY_NULL - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_ONLY_NULL_POS 0
//! Field HASH_ONLY_NULL - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_ONLY_NULL_MASK 0x1u

//! Field ENCRYPT_ONLY - No Content.
#define EIPE_EIP62_PROT_ALG_EN_ENCRYPT_ONLY_POS 1
//! Field ENCRYPT_ONLY - No Content.
#define EIPE_EIP62_PROT_ALG_EN_ENCRYPT_ONLY_MASK 0x2u

//! Field HASH_ENCRYPT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_ENCRYPT_POS 2
//! Field HASH_ENCRYPT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_ENCRYPT_MASK 0x4u

//! Field HASH_DECRYPT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_DECRYPT_POS 3
//! Field HASH_DECRYPT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_HASH_DECRYPT_MASK 0x8u

//! Field ENCRYPT_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_ENCRYPT_HASH_POS 4
//! Field ENCRYPT_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_ENCRYPT_HASH_MASK 0x10u

//! Field DECRYPT_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_DECRYPT_HASH_POS 5
//! Field DECRYPT_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_DECRYPT_HASH_MASK 0x20u

//! Field KEY_128_BIT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_KEY_128_BIT_POS 6
//! Field KEY_128_BIT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_KEY_128_BIT_MASK 0x40u

//! Field KEY_256_BIT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_KEY_256_BIT_POS 7
//! Field KEY_256_BIT - No Content.
#define EIPE_EIP62_PROT_ALG_EN_KEY_256_BIT_MASK 0x80u

//! Field AES_ECB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_ECB_POS 8
//! Field AES_ECB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_ECB_MASK 0x100u

//! Field AES_CBC - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CBC_POS 9
//! Field AES_CBC - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CBC_MASK 0x200u

//! Field AES_CTR_ICM - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CTR_ICM_POS 10
//! Field AES_CTR_ICM - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CTR_ICM_MASK 0x400u

//! Field AES_OFB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_OFB_POS 11
//! Field AES_OFB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_OFB_MASK 0x800u

//! Field AES_CFB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CFB_POS 12
//! Field AES_CFB - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_CFB_MASK 0x1000u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_PROT_ALG_EN_RESERVED_0_POS 13
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_PROT_ALG_EN_RESERVED_0_MASK 0x3FFFE000u

//! Field AES_XCBC_MAC - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_XCBC_MAC_POS 30
//! Field AES_XCBC_MAC - No Content.
#define EIPE_EIP62_PROT_ALG_EN_AES_XCBC_MAC_MASK 0x40000000u

//! Field GCM_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_GCM_HASH_POS 31
//! Field GCM_HASH - No Content.
#define EIPE_EIP62_PROT_ALG_EN_GCM_HASH_MASK 0x80000000u

//! @}

//! \defgroup EIPE_EIP62_CONTEXT_CTRL Register EIPE_EIP62_CONTEXT_CTRL - EIPE EIP62 CONTEXT CTRL
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_CONTEXT_CTRL 0x8
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_CONTEXT_CTRL 0x00B2F408u

//! Register Reset Value
#define EIPE_EIP62_CONTEXT_CTRL_RST 0xE5880212u

//! Field CONTEXT_SIZE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_CONTEXT_SIZE_POS 0
//! Field CONTEXT_SIZE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_CONTEXT_SIZE_MASK 0x1Fu

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_RESERVED_0_POS 5
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_RESERVED_0_MASK 0xE0u

//! Field ADDRESS_MODE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_ADDRESS_MODE_POS 8
//! Field ADDRESS_MODE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_ADDRESS_MODE_MASK 0x100u

//! Field CONTROL_MODE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_CONTROL_MODE_POS 9
//! Field CONTROL_MODE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_CONTROL_MODE_MASK 0x200u

//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_RESERVED_1_POS 10
//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_RESERVED_1_MASK 0xFC00u

//! Field MACSEC_ETHERTYPE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_MACSEC_ETHERTYPE_POS 16
//! Field MACSEC_ETHERTYPE - No Content.
#define EIPE_EIP62_CONTEXT_CTRL_MACSEC_ETHERTYPE_MASK 0xFFFF0000u

//! @}

//! \defgroup EIPE_EIP62_CONTEXT_STAT Register EIPE_EIP62_CONTEXT_STAT - EIPE EIP62 CONTEXT STAT
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_CONTEXT_STAT 0xC
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_CONTEXT_STAT 0x00B2F40Cu

//! Register Reset Value
#define EIPE_EIP62_CONTEXT_STAT_RST 0x00000000u

//! Field ERROR - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ERROR_POS 0
//! Field ERROR - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ERROR_MASK 0x7FFFu

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESERVED_0_POS 15
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESERVED_0_MASK 0x8000u

//! Field ACTIVE_TOKENS - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ACTIVE_TOKENS_POS 16
//! Field ACTIVE_TOKENS - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ACTIVE_TOKENS_MASK 0x30000u

//! Field ACTIVE_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ACTIVE_CONTEXT_POS 18
//! Field ACTIVE_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ACTIVE_CONTEXT_MASK 0x40000u

//! Field NEXT_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_NEXT_CONTEXT_POS 19
//! Field NEXT_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_NEXT_CONTEXT_MASK 0x80000u

//! Field RESULT_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESULT_CONTEXT_POS 20
//! Field RESULT_CONTEXT - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESULT_CONTEXT_MASK 0x100000u

//! Field ERROR_RECOVERY - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ERROR_RECOVERY_POS 21
//! Field ERROR_RECOVERY - No Content.
#define EIPE_EIP62_CONTEXT_STAT_ERROR_RECOVERY_MASK 0x200000u

//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESERVED_1_POS 22
//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_CONTEXT_STAT_RESERVED_1_MASK 0xFC00000u

//! Field INPUT_FRAME_CURRENT_STATE - No Content.
#define EIPE_EIP62_CONTEXT_STAT_INPUT_FRAME_CURRENT_STATE_POS 28
//! Field INPUT_FRAME_CURRENT_STATE - No Content.
#define EIPE_EIP62_CONTEXT_STAT_INPUT_FRAME_CURRENT_STATE_MASK 0xF0000000u

//! @}

//! \defgroup EIPE_EIP62_INT_CTRL_STAT Register EIPE_EIP62_INT_CTRL_STAT - EIPE EIP62 INT CTRL STAT
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_INT_CTRL_STAT 0x10
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_INT_CTRL_STAT 0x00B2F410u

//! Register Reset Value
#define EIPE_EIP62_INT_CTRL_STAT_RST 0xC03F0000u

//! Field INPUT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INPUT_ERROR_POS 0
//! Field INPUT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INPUT_ERROR_MASK 0x1u

//! Field OUTPUT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTPUT_ERROR_POS 1
//! Field OUTPUT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTPUT_ERROR_MASK 0x2u

//! Field PROCESSING - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_PROCESSING_POS 2
//! Field PROCESSING - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_PROCESSING_MASK 0x4u

//! Field CONTEXT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_CONTEXT_ERROR_POS 3
//! Field CONTEXT_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_CONTEXT_ERROR_MASK 0x8u

//! Field OUTB_SEQNUM_THRESHOLD - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_THRESHOLD_POS 4
//! Field OUTB_SEQNUM_THRESHOLD - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_THRESHOLD_MASK 0x10u

//! Field OUTB_SEQNUM_ROLLOVER - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_ROLLOVER_POS 5
//! Field OUTB_SEQNUM_ROLLOVER - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_ROLLOVER_MASK 0x20u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_RESERVED_0_POS 6
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_RESERVED_0_MASK 0x3FC0u

//! Field FATAL_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_FATAL_ERROR_POS 14
//! Field FATAL_ERROR - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_FATAL_ERROR_MASK 0x4000u

//! Field INTERRUPT_OUT - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INTERRUPT_OUT_POS 15
//! Field INTERRUPT_OUT - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INTERRUPT_OUT_MASK 0x8000u

//! Field INPUT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INPUT_ERROR_EN_POS 16
//! Field INPUT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INPUT_ERROR_EN_MASK 0x10000u

//! Field OUTPUT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTPUT_ERROR_EN_POS 17
//! Field OUTPUT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTPUT_ERROR_EN_MASK 0x20000u

//! Field PROCESSING_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_PROCESSING_EN_POS 18
//! Field PROCESSING_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_PROCESSING_EN_MASK 0x40000u

//! Field CONTEXT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_CONTEXT_ERROR_EN_POS 19
//! Field CONTEXT_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_CONTEXT_ERROR_EN_MASK 0x80000u

//! Field OUTB_SEQNUM_THRESHOLD_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_THRESHOLD_EN_POS 20
//! Field OUTB_SEQNUM_THRESHOLD_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_THRESHOLD_EN_MASK 0x100000u

//! Field OUTB_SEQNUM_ROLLOVER_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_ROLLOVER_EN_POS 21
//! Field OUTB_SEQNUM_ROLLOVER_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_OUTB_SEQNUM_ROLLOVER_EN_MASK 0x200000u

//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_RESERVED_1_POS 22
//! Field RESERVED_1 - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_RESERVED_1_MASK 0x3FC00000u

//! Field FATAL_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_FATAL_ERROR_EN_POS 30
//! Field FATAL_ERROR_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_FATAL_ERROR_EN_MASK 0x40000000u

//! Field INTERRUPT_OUT_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INTERRUPT_OUT_EN_POS 31
//! Field INTERRUPT_OUT_EN - No Content.
#define EIPE_EIP62_INT_CTRL_STAT_INTERRUPT_OUT_EN_MASK 0x80000000u

//! @}

//! \defgroup EIPE_EIP62_SW_INTERRUPT Register EIPE_EIP62_SW_INTERRUPT - EIPE EIP62 SW INTERRUPT
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_SW_INTERRUPT 0x14
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_SW_INTERRUPT 0x00B2F414u

//! Register Reset Value
#define EIPE_EIP62_SW_INTERRUPT_RST 0x00000000u

//! Field SW_INTERRUPT - No Content.
#define EIPE_EIP62_SW_INTERRUPT_SW_INTERRUPT_POS 0
//! Field SW_INTERRUPT - No Content.
#define EIPE_EIP62_SW_INTERRUPT_SW_INTERRUPT_MASK 0x7FFFFFFFu

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_SW_INTERRUPT_RESERVED_0_POS 31
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_SW_INTERRUPT_RESERVED_0_MASK 0x80000000u

//! @}

//! \defgroup EIPE_EIP62_SEQ_NUM_THRESHOLD Register EIPE_EIP62_SEQ_NUM_THRESHOLD - EIPE EIP62 SEQ NUM THRESHOLD
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_SEQ_NUM_THRESHOLD 0x20
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_SEQ_NUM_THRESHOLD 0x00B2F420u

//! Register Reset Value
#define EIPE_EIP62_SEQ_NUM_THRESHOLD_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_SEQ_NUM_THRESHOLD64_LO Register EIPE_EIP62_SEQ_NUM_THRESHOLD64_LO - EIPE EIP62 SEQ NUM THRESHOLD64 LO
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_SEQ_NUM_THRESHOLD64_LO 0x24
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_SEQ_NUM_THRESHOLD64_LO 0x00B2F424u

//! Register Reset Value
#define EIPE_EIP62_SEQ_NUM_THRESHOLD64_LO_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_SEQ_NUM_THRESHOLD64_HI Register EIPE_EIP62_SEQ_NUM_THRESHOLD64_HI - EIPE EIP62 SEQ NUM THRESHOLD64 HI
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_SEQ_NUM_THRESHOLD64_HI 0x28
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_SEQ_NUM_THRESHOLD64_HI 0x00B2F428u

//! Register Reset Value
#define EIPE_EIP62_SEQ_NUM_THRESHOLD64_HI_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_CONTEXT_UPD_CTRL Register EIPE_EIP62_CONTEXT_UPD_CTRL - EIPE EIP62 CONTEXT UPD CTRL
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_CONTEXT_UPD_CTRL 0x30
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_CONTEXT_UPD_CTRL 0x00B2F430u

//! Register Reset Value
#define EIPE_EIP62_CONTEXT_UPD_CTRL_RST 0x00000000u

//! Field BLOCK_CONTEXT_UPDATES - No Content.
#define EIPE_EIP62_CONTEXT_UPD_CTRL_BLOCK_CONTEXT_UPDATES_POS 0
//! Field BLOCK_CONTEXT_UPDATES - No Content.
#define EIPE_EIP62_CONTEXT_UPD_CTRL_BLOCK_CONTEXT_UPDATES_MASK 0x3u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_UPD_CTRL_RESERVED_0_POS 2
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_CONTEXT_UPD_CTRL_RESERVED_0_MASK 0xFFFFFFFCu

//! @}

//! \defgroup EIPE_EIP62_NEXTPN_LO Register EIPE_EIP62_NEXTPN_LO - EIPE EIP62 NEXTPN LO
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_NEXTPN_LO 0x80
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_NEXTPN_LO 0x00B2F480u

//! Register Reset Value
#define EIPE_EIP62_NEXTPN_LO_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_NEXTPN_HI Register EIPE_EIP62_NEXTPN_HI - EIPE EIP62 NEXTPN HI
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_NEXTPN_HI 0x84
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_NEXTPN_HI 0x00B2F484u

//! Register Reset Value
#define EIPE_EIP62_NEXTPN_HI_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_NEXTPN_CTX_ID Register EIPE_EIP62_NEXTPN_CTX_ID - EIPE EIP62 NEXTPN CTX ID
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_NEXTPN_CTX_ID 0x88
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_NEXTPN_CTX_ID 0x00B2F488u

//! Register Reset Value
#define EIPE_EIP62_NEXTPN_CTX_ID_RST 0x00000000u

//! @}

//! \defgroup EIPE_EIP62_NEXTPN_CTRL Register EIPE_EIP62_NEXTPN_CTRL - EIPE EIP62 NEXTPN CTRL
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_NEXTPN_CTRL 0x8C
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_NEXTPN_CTRL 0x00B2F48Cu

//! Register Reset Value
#define EIPE_EIP62_NEXTPN_CTRL_RST 0x00000000u

//! Field ENABLE_UPDATE - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_ENABLE_UPDATE_POS 0
//! Field ENABLE_UPDATE - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_ENABLE_UPDATE_MASK 0x1u

//! Field NEXTPN_WRITTEN - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_NEXTPN_WRITTEN_POS 1
//! Field NEXTPN_WRITTEN - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_NEXTPN_WRITTEN_MASK 0x2u

//! Field SEQUENCE_NUMBER_SIZE - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_SEQUENCE_NUMBER_SIZE_POS 2
//! Field SEQUENCE_NUMBER_SIZE - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_SEQUENCE_NUMBER_SIZE_MASK 0x4u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_RESERVED_0_POS 3
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_RESERVED_0_MASK 0x8u

//! Field NEXTPN_CONTEXT_ADDRESS - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_NEXTPN_CONTEXT_ADDRESS_POS 4
//! Field NEXTPN_CONTEXT_ADDRESS - No Content.
#define EIPE_EIP62_NEXTPN_CTRL_NEXTPN_CONTEXT_ADDRESS_MASK 0xFFFFFFF0u

//! @}

//! \defgroup EIPE_EIP62_UPDATE_CTRL Register EIPE_EIP62_UPDATE_CTRL - EIPE EIP62 UPDATE CTRL
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_UPDATE_CTRL 0xC0
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_UPDATE_CTRL 0x00B2F4C0u

//! Register Reset Value
#define EIPE_EIP62_UPDATE_CTRL_RST 0x0000000Du

//! Field UPDATE_THRESHOLD - No Content.
#define EIPE_EIP62_UPDATE_CTRL_UPDATE_THRESHOLD_POS 0
//! Field UPDATE_THRESHOLD - No Content.
#define EIPE_EIP62_UPDATE_CTRL_UPDATE_THRESHOLD_MASK 0xFFu

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_UPDATE_CTRL_RESERVED_0_POS 8
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_UPDATE_CTRL_RESERVED_0_MASK 0x7FFFFF00u

//! Field THRESHOLD_EN - No Content.
#define EIPE_EIP62_UPDATE_CTRL_THRESHOLD_EN_POS 31
//! Field THRESHOLD_EN - No Content.
#define EIPE_EIP62_UPDATE_CTRL_THRESHOLD_EN_MASK 0x80000000u

//! @}

//! \defgroup EIPE_EIP62_TYPE_REG Register EIPE_EIP62_TYPE_REG - EIPE EIP62 TYPE REG
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_TYPE_REG 0x3F8
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_TYPE_REG 0x00B2F7F8u

//! Register Reset Value
#define EIPE_EIP62_TYPE_REG_RST 0x44285C21u

//! Field MINOR_VERSION - No Content.
#define EIPE_EIP62_TYPE_REG_MINOR_VERSION_POS 0
//! Field MINOR_VERSION - No Content.
#define EIPE_EIP62_TYPE_REG_MINOR_VERSION_MASK 0xFu

//! Field MAJOR_VERSION - No Content.
#define EIPE_EIP62_TYPE_REG_MAJOR_VERSION_POS 4
//! Field MAJOR_VERSION - No Content.
#define EIPE_EIP62_TYPE_REG_MAJOR_VERSION_MASK 0xF0u

//! Field FPGA_SOLUTION - No Content.
#define EIPE_EIP62_TYPE_REG_FPGA_SOLUTION_POS 8
//! Field FPGA_SOLUTION - No Content.
#define EIPE_EIP62_TYPE_REG_FPGA_SOLUTION_MASK 0x100u

//! Field ASIC_AES_GF_SBOX - No Content.
#define EIPE_EIP62_TYPE_REG_ASIC_AES_GF_SBOX_POS 9
//! Field ASIC_AES_GF_SBOX - No Content.
#define EIPE_EIP62_TYPE_REG_ASIC_AES_GF_SBOX_MASK 0x200u

//! Field ASIC_AES_LU_SBOX - No Content.
#define EIPE_EIP62_TYPE_REG_ASIC_AES_LU_SBOX_POS 10
//! Field ASIC_AES_LU_SBOX - No Content.
#define EIPE_EIP62_TYPE_REG_ASIC_AES_LU_SBOX_MASK 0x400u

//! Field MACSEC_AES_ONLY - No Content.
#define EIPE_EIP62_TYPE_REG_MACSEC_AES_ONLY_POS 11
//! Field MACSEC_AES_ONLY - No Content.
#define EIPE_EIP62_TYPE_REG_MACSEC_AES_ONLY_MASK 0x800u

//! Field AES - No Content.
#define EIPE_EIP62_TYPE_REG_AES_POS 12
//! Field AES - No Content.
#define EIPE_EIP62_TYPE_REG_AES_MASK 0x1000u

//! Field AES_FB - No Content.
#define EIPE_EIP62_TYPE_REG_AES_FB_POS 13
//! Field AES_FB - No Content.
#define EIPE_EIP62_TYPE_REG_AES_FB_MASK 0x2000u

//! Field AES_SPEED - No Content.
#define EIPE_EIP62_TYPE_REG_AES_SPEED_POS 14
//! Field AES_SPEED - No Content.
#define EIPE_EIP62_TYPE_REG_AES_SPEED_MASK 0x3C000u

//! Field AES192 - No Content.
#define EIPE_EIP62_TYPE_REG_AES192_POS 18
//! Field AES192 - No Content.
#define EIPE_EIP62_TYPE_REG_AES192_MASK 0x40000u

//! Field AES256 - No Content.
#define EIPE_EIP62_TYPE_REG_AES256_POS 19
//! Field AES256 - No Content.
#define EIPE_EIP62_TYPE_REG_AES256_MASK 0x80000u

//! Field EOP_PARAM_BITS - No Content.
#define EIPE_EIP62_TYPE_REG_EOP_PARAM_BITS_POS 20
//! Field EOP_PARAM_BITS - No Content.
#define EIPE_EIP62_TYPE_REG_EOP_PARAM_BITS_MASK 0x1F00000u

//! Field IPSEC - No Content.
#define EIPE_EIP62_TYPE_REG_IPSEC_POS 25
//! Field IPSEC - No Content.
#define EIPE_EIP62_TYPE_REG_IPSEC_MASK 0x2000000u

//! Field HDR_EXTENSIONS - No Content.
#define EIPE_EIP62_TYPE_REG_HDR_EXTENSIONS_POS 26
//! Field HDR_EXTENSIONS - No Content.
#define EIPE_EIP62_TYPE_REG_HDR_EXTENSIONS_MASK 0x4000000u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_TYPE_REG_RESERVED_0_POS 27
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_TYPE_REG_RESERVED_0_MASK 0x38000000u

//! Field GHASH_AVAILABLE - No Content.
#define EIPE_EIP62_TYPE_REG_GHASH_AVAILABLE_POS 30
//! Field GHASH_AVAILABLE - No Content.
#define EIPE_EIP62_TYPE_REG_GHASH_AVAILABLE_MASK 0x40000000u

//! Field GHASH_SPEED - No Content.
#define EIPE_EIP62_TYPE_REG_GHASH_SPEED_POS 31
//! Field GHASH_SPEED - No Content.
#define EIPE_EIP62_TYPE_REG_GHASH_SPEED_MASK 0x80000000u

//! @}

//! \defgroup EIPE_EIP62_VERSION Register EIPE_EIP62_VERSION - EIPE EIP62 VERSION
//! @{

//! Register Offset (relative)
#define EIPE_EIP62_VERSION 0x3FC
//! Register Offset (absolute) for 1st Instance EIPE_EIP160_EIP62_EIP62_REGISTERS
#define EIPE_EIP160_EIP62_EIP62_REGISTERS_EIPE_EIP62_VERSION 0x00B2F7FCu

//! Register Reset Value
#define EIPE_EIP62_VERSION_RST 0x0214C13Eu

//! Field EIP_NUMBER - No Content.
#define EIPE_EIP62_VERSION_EIP_NUMBER_POS 0
//! Field EIP_NUMBER - No Content.
#define EIPE_EIP62_VERSION_EIP_NUMBER_MASK 0xFFu

//! Field EIP_NUMBER_COMPL - No Content.
#define EIPE_EIP62_VERSION_EIP_NUMBER_COMPL_POS 8
//! Field EIP_NUMBER_COMPL - No Content.
#define EIPE_EIP62_VERSION_EIP_NUMBER_COMPL_MASK 0xFF00u

//! Field PATCH_LEVEL - No Content.
#define EIPE_EIP62_VERSION_PATCH_LEVEL_POS 16
//! Field PATCH_LEVEL - No Content.
#define EIPE_EIP62_VERSION_PATCH_LEVEL_MASK 0xF0000u

//! Field MINOR_VERSION - No Content.
#define EIPE_EIP62_VERSION_MINOR_VERSION_POS 20
//! Field MINOR_VERSION - No Content.
#define EIPE_EIP62_VERSION_MINOR_VERSION_MASK 0xF00000u

//! Field MAJOR_VERSION - No Content.
#define EIPE_EIP62_VERSION_MAJOR_VERSION_POS 24
//! Field MAJOR_VERSION - No Content.
#define EIPE_EIP62_VERSION_MAJOR_VERSION_MASK 0xF000000u

//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_VERSION_RESERVED_0_POS 28
//! Field RESERVED_0 - No Content.
#define EIPE_EIP62_VERSION_RESERVED_0_MASK 0xF0000000u

//! @}

//! @}

#endif
