
MEPA_DRV(LIB_NAME mepa_drv_ksz9031
         DEFS     MEPA_HAS_KSZ9031
         SRCS     phy_driver.c
         INCL_PUB ${CMAKE_SOURCE_DIR}/me/include
                  ${MEPA_SOURCE_DIR}/include
)

option(MEPA_ksz9031 "Add KSZ9031 phy support in libmeba" OFF)
if (${MEPA_ksz9031})
    set(mepa_drvs ${mepa_drvs} mepa_drv_ksz9031 CACHE INTERNAL "")
    set(mepa_defs ${mepa_defs} -DMEPA_HAS_KSZ9031 CACHE INTERNAL "")
endif()
