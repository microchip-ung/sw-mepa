#Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
#SPDX-License-Identifier: MIT

file(GLOB src_lan80xx
    "src/*.c"
)

file(GLOB src_sha256
    "src/sha256/*.c"
)

set(sha256_defs -DMBEDTLS_CONFIG_FILE="sha256_config.h")
set(lan80xx_ts_defs -DLAN80XX_A0_FW_WORKAROUND=1)
set(lan80xx_ts_msec_defs -DLAN80XX_A0_FW_WORKAROUND=1 -DMEPA_LAN80XX_MSEC=1)

option(MEPA_lan80xx_DFU_PROFILING "Add DFU profiling support for mepa lan80xx" OFF)

if (${MEPA_lan80xx_DFU_PROFILING})
    set(lan80xx_ts_defs ${lan80xx_ts_defs} -DDFU_PROFILING_EN)
endif()

MEPA_DRV(LIB_NAME mepa_drv_lan80xx_ts
    DEFS MEPA_HAS_LAN80XX ${lan80xx_ts_defs} ${sha256_defs}
    SRCS  ${src_lan80xx} ${src_sha256}
    INCL_PUB ${CMAKE_SOURCE_DIR}/sw-mesa/me/include
             ${CMAKE_SOURCE_DIR}/me/include
             ${MEPA_SOURCE_DIR}/include 
             ${MEPA_SOURCE_DIR}/vtss/include
    INCL_PRI ${CMAKE_CURRENT_SOURCE_DIR}/src/regs
             ${CMAKE_CURRENT_SOURCE_DIR}/include/microchip
             ${CMAKE_CURRENT_SOURCE_DIR}/src
)

MEPA_DRV(LIB_NAME mepa_drv_lan80xx_ts_msec
    DEFS MEPA_HAS_LAN80XX ${lan80xx_ts_msec_defs} ${sha256_defs}
    SRCS  ${src_lan80xx} ${src_sha256}
    INCL_PUB ${CMAKE_SOURCE_DIR}/sw-mesa/me/include
             ${CMAKE_SOURCE_DIR}/me/include
             ${MEPA_SOURCE_DIR}/include
             ${MEPA_SOURCE_DIR}/vtss/include
    INCL_PRI ${CMAKE_CURRENT_SOURCE_DIR}/src/regs
             ${CMAKE_CURRENT_SOURCE_DIR}/include/microchip
             ${CMAKE_CURRENT_SOURCE_DIR}/src)

option(MEPA_lan80xx_ts "Add lan80xx support in libmeba" OFF)
if (${MEPA_lan80xx_ts})
    set(mepa_drvs ${mepa_drvs} mepa_drv_lan80xx_ts CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_LAN80XX CACHE INTERNAL "")
endif()

option(MEPA_lan80xx_ts_msec "Add lan80xx support in libmeba with MSEC" OFF)
if (${MEPA_lan80xx_ts_msec})
    set(mepa_drvs ${mepa_drvs} mepa_drv_lan80xx_ts_msec CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_LAN80XX CACHE INTERNAL "")
endif()

add_library(mepa_lan80xx_headers INTERFACE)
target_sources(mepa_lan80xx_headers
    PUBLIC
        FILE_SET HEADERS
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/microchip/
)
install(
    TARGETS mepa_lan80xx_headers
    EXPORT mepaTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mepa/microchip/
)

foreach(_tgt mepa_drv_lan80xx_ts mepa_drv_lan80xx_ts)
    target_include_directories(${_tgt} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mepa/microchip>)
endforeach()