MEPA_DRV(LIB_NAME mepa_drv_lan8814
         DEFS     MEPA_HAS_LAN8814
         SRCS     src/lan8814.c
                  src/lan8814_ts.c
         INCL_PUB ${CMAKE_SOURCE_DIR}/me/include
                  ${MEPA_SOURCE_DIR}/include
                  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

MEPA_DRV(LIB_NAME mepa_drv_lan8814_light
         DEFS     MEPA_HAS_LAN8814 -DMEPA_LAN8814_LIGHT=1
         SRCS     src/lan8814.c
                  src/lan8814_ts.c
         INCL_PUB ${CMAKE_SOURCE_DIR}/me/include
                  ${MEPA_SOURCE_DIR}/include
                  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

option(MEPA_lan8814 "Add LAN8814 support in libmeba" OFF)
if (${MEPA_lan8814})
    set(mepa_drvs ${mepa_drvs} mepa_drv_lan8814 CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_LAN8814 CACHE INTERNAL "")
endif()

option(MEPA_lan8814_light "Add LAN8814-light support in libmeba" OFF)
if (${MEPA_lan8814_light})
    set(mepa_drvs ${mepa_drvs} MEPA_lan8814_light CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_LAN8814 CACHE INTERNAL "")
endif()

if (${MEPA_lan8814} AND ${MEPA_lan8814_light})
    message(FATAL_ERROR "MEPA_lan8814 and MEPA_lan8814_light are mutually exclusive")
endif()

add_library(mepa_lan8814_headers INTERFACE)
target_sources(mepa_lan8814_headers
    PUBLIC
        FILE_SET HEADERS
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/microchip/
)
install(
    TARGETS mepa_lan8814_headers
    EXPORT mepaTargets
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mepa/microchip/
)

foreach(_tgt mepa_drv_lan8814 mepa_drv_lan8814_light)
    target_include_directories(${_tgt} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/mepa/microchip>)
endforeach()