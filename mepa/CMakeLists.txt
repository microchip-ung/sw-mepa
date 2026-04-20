# Copyright (c) 2004-2021 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

project(MEPA
    LANGUAGES C ASM
)

cmake_minimum_required(VERSION 3.15)
include(.cmake/mepa-macros.cmake)

include_directories(common/include)

# Offer an options for CI to get good compliation coverage
option(BUILD_MEPA_ALL "Build all MEPA libraries" OFF)
option(MEPA_OPSYS_VELOCITYSP "Build MEPA with OS abstraction for VELOCITYSP" OFF)

mark_as_advanced(BUILD_MEPA_ALL)
mark_as_advanced(MEPA_OPSYS_VELOCITYSP)

# The various drivers append to this, if user select the corresponding options
set(mepa_drvs)
set(mepa_defs)

# Pull in all drivers (alfabetic sorted)
add_subdirectory(aqr)
add_subdirectory(intel)
add_subdirectory(ksz9031)
add_subdirectory(microchip)
add_subdirectory(vtss)
add_subdirectory(dummy_phy)


###############################################################################
## This is the "default" MEPA packet which can be customized using the menus ##
###############################################################################
MEPA_LIB(LIB_NAME mepa
         DEFS     ${mepa_defs}
         DRVS     ${mepa_drvs})


###############################################################################
## Collection of pre-defined MEPA packets. These packets are used by MEBA    ##
## and other higher layer of applications                                    ##
###############################################################################
MEPA_LIB(LIB_NAME mepa_caracal ADVANCED
         DRVS     mepa_drv_vtss)

MEPA_LIB(LIB_NAME mepa_caracal_macsec ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts)

MEPA_LIB(LIB_NAME mepa_jr2 ADVANCED
         DRVS     mepa_drv_vtss_10g_ts mepa_drv_aqr mepa_drv_intel)

MEPA_LIB(LIB_NAME mepa_jr2_macsec ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts mepa_drv_aqr mepa_drv_intel)

MEPA_LIB(LIB_NAME mepa_jr2_bringup ADVANCED
         DRVS     mepa_drv_vtss_10g_ts)

MEPA_LIB(LIB_NAME mepa_servalt ADVANCED
         DRVS     mepa_drv_vtss_10g_ts)

MEPA_LIB(LIB_NAME mepa_servalt_macsec ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts)

MEPA_LIB(LIB_NAME mepa_ocelot ADVANCED
         DRVS     mepa_drv_vtss_ts mepa_drv_lan8814)

MEPA_LIB(LIB_NAME mepa_ocelot_macsec ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts mepa_drv_lan8814)

MEPA_LIB(LIB_NAME mepa_ocelot_pcb121 ADVANCED
         DRVS     mepa_drv_vtss_ts)

MEPA_LIB(LIB_NAME mepa_sparx5 ADVANCED
         DRVS     mepa_drv_vtss_10g_ts mepa_drv_aqr mepa_drv_intel mepa_drv_lan8814 mepa_drv_lan884x)

MEPA_LIB(LIB_NAME mepa_sparx5_macsec ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts mepa_drv_aqr mepa_drv_intel mepa_drv_lan8814)

MEPA_LIB(LIB_NAME mepa_sparx5_bringup ADVANCED
         DRVS     mepa_drv_vtss_10g_ts)

MEPA_LIB(LIB_NAME mepa_edsx ADVANCED
         DRVS     mepa_drv_vtss_10g_macsec_ts mepa_drv_aqr mepa_drv_intel mepa_drv_lan8814 mepa_drv_lan80xx_ts_msec)

MEPA_LIB(LIB_NAME mepa_lan966x ADVANCED
         DRVS     mepa_drv_vtss mepa_drv_ksz9031 mepa_drv_lan8814 mepa_drv_lan884x)

MEPA_LIB(LIB_NAME mepa_eds2 ADVANCED
         DRVS     mepa_drv_vtss_ts mepa_drv_ksz9031 mepa_drv_lan8814 mepa_drv_lan884x)

MEPA_LIB(LIB_NAME mepa_velocitysp_lan966x ADVANCED
         DRVS     mepa_drv_lan8814)

MEPA_LIB(LIB_NAME mepa_velocitysp_lan969x ADVANCED
         DRVS     mepa_drv_lan8814_light mepa_drv_lan884x mepa_drv_lan8x8x)

MEPA_LIB(LIB_NAME mepa_velocitysp_lan969x_auto ADVANCED
         DRVS     mepa_drv_lan887x mepa_drv_lan884x mepa_drv_lan867x mepa_drv_lan8x8x)

MEPA_LIB(LIB_NAME mepa_p64h ADVANCED
         DRVS     mepa_drv_vtss_10g_ts mepa_drv_dummy_phy)

install(
    DIRECTORY common
    DESTINATION ${CMAKE_INSTALL_DATADIR}/mepa
)
