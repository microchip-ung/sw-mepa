MEPA_DRV(LIB_NAME mepa_drv_aqr
         SRCS     phy_driver.c
                  vtss_platform/AQ_PhyInterface.c
                  vtss_platform/AQ_PlatformRoutines.c
                  AQR_API_2.6.1/src/AQ_API.c
                  AQR_API_2.6.1/src/AQ_API_flash.c
         INCL_PUB ${CMAKE_SOURCE_DIR}/me/include
                  ${MEPA_SOURCE_DIR}/include
         INCL_PRI ${CMAKE_CURRENT_SOURCE_DIR}/AQR_API_2.6.1/include
                  ${CMAKE_CURRENT_SOURCE_DIR}/AQR_API_2.6.1/include/registerMap
                  ${CMAKE_CURRENT_SOURCE_DIR}/AQR_API_2.6.1/include/registerMap/APPIA
                  ${CMAKE_CURRENT_SOURCE_DIR}/AQR_API_2.6.1/include/registerMap/EUR
                  ${CMAKE_CURRENT_SOURCE_DIR}/AQR_API_2.6.1/include/registerMap/HHD
                  ${CMAKE_CURRENT_SOURCE_DIR}/aqr_phy_fw)

option(MEPA_aqr "Add AQR family support in libmeba" OFF)
if (${MEPA_aqr})
    set(mepa_drvs ${mepa_drvs} mepa_drv_aqr CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_AQR CACHE INTERNAL "")
endif()

