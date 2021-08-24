// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#ifndef _VTSS_MALIBU_B_F2DF_DF2F__API_SD10G65_H_UTE
#define _VTSS_MALIBU_B_F2DF_DF2F__API_SD10G65_H_UTE


/* Generation Tag is  UTE_release_vts_ute_tcllib_20180312_trunk_bjo */

/* ================================================================= *
 *               Note: This code is autogenerated from the
 *                     Universal Tcl Environment (UTE) which provides
 *                     consistent setup functions for
 *       ^             - Verification
 *      / \            - Validation
 *     /   \           - Test Pattern Generation and
 *    /  |  \          - Software
 *   /   !   \         It should not be modified manually.
 *  /_________\        In case there is a need for modifications,
 *                     please contact
 *                       Patrick Urban <patrick.urban@microsemi.com> or
 *                       Alexander Koch <alexander.koch@microsemi.com> or
 *                       Mark Venneboerger <mark.venneboerger@microsemi.com>
 *                     Please use Bugzilla for reporting issues or requesting enhancements:
 *                     Bugzilla: Tools->Victoria
 *                     http://projissuetracker/bugzilla/enter_bug.cgi?product=Victoria
 * ================================================================= */

#include <vtss_phy_options.h>
#if defined(VTSS_ARCH_MALIBU_B)

vtss_rc vtss_malibu_b_f2df_df2f_apc10g_setup(vtss_state_t *vtss_state, const vtss_sd10g65_setup_apc_args_t config, vtss_port_no_t port_no);
vtss_rc vtss_malibu_b_f2df_df2f_sd10g_setup_df2f(vtss_state_t *vtss_state, const vtss_sd10g65_setup_df2f_args_t config, vtss_port_no_t port_no);
vtss_rc vtss_malibu_b_f2df_df2f_sd10g_setup_f2df(vtss_state_t *vtss_state, const vtss_sd10g65_setup_f2df_args_t config, vtss_port_no_t port_no);
#endif
#endif
