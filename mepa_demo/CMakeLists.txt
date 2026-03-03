# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

message(STATUS "CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'")

option(BUILD_MESA_APP_ALL "Build all defined mesa demo applications" OFF)
option(BUILD_MESA_IMG_ALL "Build all defined mesa demo firmware images" OFF)
mark_as_advanced(BUILD_MESA_APP_ALL)

find_library(JSON_LIB json-c)
if(NOT JSON_LIB)
      message(FATAL_ERROR "json-c not found")
endif()
add_library(json-c SHARED IMPORTED GLOBAL) # GLOBAL -> if outside src tree
set_property(TARGET json-c PROPERTY IMPORTED_LOCATION ${JSON_LIB})

add_executable(base64decode base64decode.c)

add_executable(cli cmd.c)
set_target_properties(cli PROPERTIES OUTPUT_NAME "mepa-cmd")
add_library(btsig SHARED btsig.c)

add_executable(er easy_run.c)
add_executable(spi_reg_read spi_reg_read.c)

add_executable(init initramfs/main.c)
target_link_libraries(init PUBLIC "-static")

# TODO, delete these once we do not need them any more
add_executable(fa_board_fpga_init fa_board_fpga_init.c)

include_directories(../board-configs/include)
include_directories(../mepa/vtss/include)
include_directories(../mepa/vtss/src)
include_directories(../mepa/microchip/lan80xx/include/microchip)
include_directories(../mepa/microchip/lan80xx/src)
include_directories(../mepa/microchip/lan8814/include)
include_directories(../mepa/common/include)


if (EXISTS ag/mesa-rpc.c AND EXISTS ag/mesa-rpc.h AND ${MESA_PRE_AG})
set(MESA_RPC ag/mesa-rpc.c)
include_directories(ag)
else()
set(API_MEBA_CACHE_IN)
list(APPEND API_MEBA_CACHE_IN board-configs/include/microchip/ethernet/board/api/types.h)
list(APPEND API_MEBA_CACHE_IN board-configs/include/microchip/ethernet/board/api/phy.h)
list(APPEND API_MEBA_CACHE_IN board-configs/include/microchip/ethernet/board/api/phy_macsec.h)
list(APPEND API_MEBA_CACHE_IN board-configs/include/microchip/ethernet/board/api/phy_ts.h)
list(APPEND API_MEBA_CACHE_IN mepa/include/microchip/ethernet/phy/api/types.h)
list(APPEND API_MEBA_CACHE_IN mepa/include/microchip/ethernet/phy/api/phy.h)
list(APPEND API_MEBA_CACHE_IN mepa/include/microchip/ethernet/phy/api/phy_macsec.h)
list(APPEND API_MEBA_CACHE_IN mepa/include/microchip/ethernet/phy/api/phy_ts.h)
list(APPEND API_MEBA_CACHE_IN mepa/vtss/include/vtss_phy_types.h)
list(APPEND API_MEBA_CACHE_IN mepa/vtss/include/vtss_phy_1g_api.h)
list(APPEND API_MEBA_CACHE_IN mepa/vtss/include/vtss_phy_10g_api.h)
list(APPEND API_MEBA_CACHE_IN mepa/microchip/lan80xx/include/microchip/lan80xx.h)
list(APPEND API_MEBA_CACHE_IN mepa/microchip/lan80xx/src/lan80xx_types.h)
list(APPEND API_MEBA_CACHE_IN mepa/microchip/lan80xx/include/microchip/lan80xx_ts.h)
list(APPEND API_MEBA_CACHE_IN mepa/microchip/lan80xx/src/lan80xx_ts_private.h)
list(APPEND API_MEBA_CACHE_IN mepa/microchip/lan80xx/src/lan80xx_macsec_private.h)
set(API_MESA_CACHE_AG_IN)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/me/include/microchip/ethernet/common.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/afi.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/capability.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/clock.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/init.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/l2.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/l3.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/misc.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/mrp.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/packet.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/port.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/qos.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/security.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/synce.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/ts.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/tsn.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/types.h)
list (APPEND API_MESA_CACHE_AG_IN sw-mesa/mesa/include/microchip/ethernet/switch/api/vop.h)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa-rpc.h ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa-rpc.c
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ag2.rb -b ${CMAKE_CURRENT_SOURCE_DIR}/.. -o ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag ${API_MESA_CACHE_AG_IN} ${API_MEBA_CACHE_IN} 
                   DEPENDS mesa_cache
                   COMMENT "Generate mesa-rpc.h/mesa-rpc.c")
add_custom_target(mesa_rpc_src DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa-rpc.h ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa-rpc.c)
set(MESA_RPC ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa-rpc.c)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/mesa-ag)
endif()

include_directories(.)

file(GLOB_RECURSE MEPA_APP_SRC
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     #RELATIVE ""
     "${CMAKE_CURRENT_SOURCE_DIR}/mepa_apps/*.c")


add_library(mepa_demo_lib STATIC trace.c cli.c port.c mac.c vlan.c packet.c ip.c
                                 debug.c symreg.c test.c spi.c uio.c ${MESA_RPC} json_rpc.c
                                 example.c kr.c intr.c  ${MEPA_APP_SRC})
target_include_directories(mepa_demo_lib PUBLIC ${CMAKE_BINARY_DIR}/mesa-ag/)

file(GLOB_RECURSE EXAMPLE_SRC
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     #RELATIVE ""
     "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.c")

file(GLOB_RECURSE EXAMPLE_SRC_MEPA
     LIST_DIRECTORIES false
     CONFIGURE_DEPENDS
     #RELATIVE ""
     "${CMAKE_CURRENT_SOURCE_DIR}/../../mepa/demo/examples/*.c")

add_library(mepa_demo_examples_lib INTERFACE)
target_sources(mepa_demo_examples_lib INTERFACE ${EXAMPLE_SRC} ${EXAMPLE_SRC_MEPA})
#target_include_directories(mepa_demo_examples_lib PUBLIC ${CMAKE_BINARY_DIR}/mesa-ag/)

add_library(chip_specific_api_stub STATIC ../mepa/microchip/lan_stubs.c)

target_include_directories(chip_specific_api_stub PUBLIC ../mepa/include ../mepa/microchip/lan80xx/include)

macro(APP_TARGET)
    set(options )
    set(oneValueArgs NAME MESA MEBA)
    set(multiValueArgs APP_TARGET_DEFINES)
    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    option(${A_NAME} "Build the MESA DEMO application for ${MESA}" OFF)
    mark_as_advanced(${A_NAME})
    if (${BUILD_MESA_APP_ALL} OR ${BUILD_ALL})
        set(${A_NAME} ON CACHE BOOL "" FORCE)
    endif()

    add_executable(${A_NAME} main.c)
    target_link_libraries(${A_NAME}
                mepa_demo_lib
                mepa_demo_examples_lib
                json-c
                ${A_MESA}
                ${A_MEBA}_static
                chip_specific_api_stub
            )
    target_compile_options(mepa_demo_lib PUBLIC ${A_APP_TARGET_DEFINES})
    target_compile_options(mepa_demo_examples_lib INTERFACE ${A_APP_TARGET_DEFINES})
    set_target_properties(${A_NAME} PROPERTIES OUTPUT_NAME "mesa-demo-${A_MESA}")

    if (${A_NAME})
        set_target_properties(${A_NAME} PROPERTIES EXCLUDE_FROM_ALL FALSE)
    else()
        set_target_properties(${A_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()

    install_targets(TARGETS ${A_NAME})
endmacro()


macro(IMG_TARGET)
    set(options MERA)
    set(oneValueArgs NAME APP MACHINE TYPE)
    set(multiValueArgs )
    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND A_MACHINE STREQUAL "ls1046a")
        set(KERNEL "${RELOCATED_HOST_DIR}/../mscc-linux-kernel.bin.xz")
    elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(KERNEL "${RELOCATED_HOST_DIR}/../mscc-linux-kernel.bin.gz")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l" AND A_MACHINE STREQUAL "BeagleBoneBlack")
        set(KERNEL "${RELOCATED_HOST_DIR}/../mscc-linux-kernel.bin.gz")
    endif()

    option(IMG_${A_NAME}_${A_TYPE} "Build the MESA DEMO firmware image for ${A_MESA}/${A_MEBA}" OFF)
    if (${BUILD_MESA_IMG_ALL} OR ${BUILD_ALL})
        set(IMG_${A_NAME}_${A_TYPE} ON CACHE BOOL "" FORCE)
    endif()

    # Building up the imggen.rb command
    set(cmd ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb)
    set(cmd ${cmd} -p ${CMAKE_CURRENT_SOURCE_DIR})
    set(cmd ${cmd} -n ${A_NAME})
    set(cmd ${cmd} -m ${A_MACHINE})
    set(cmd ${cmd} -b ${RELOCATED_HOST_DIR}/../../../../)
    set(cmd ${cmd} -s ${CMAKE_STRIP})

    # List of executables to install on target
    set(EXECUTABLES ${A_APP} cli er fa_board_fpga_init spi_reg_read base64decode)

    if (${A_MERA})
        set(EXECUTABLES ${EXECUTABLES} mera-demo mera-cmd mera-sram-rw)
    endif ()

    # Add library dependencies
    set(SOLIBS  "")

    # Find all the libraries needed by the list of executable
    foreach(E ${EXECUTABLES})
        set(cmd ${cmd} -i $<TARGET_FILE:${E}>)

        get_target_property(DEPS ${E} LINK_LIBRARIES)
        foreach(dep ${DEPS})
            #message(STATUS "${E}: ${dep}")
            if (NOT ${dep} STREQUAL "DEPS-NOTFOUND")
                get_target_property(dep_type ${dep} TYPE)
                if(${dep_type} STREQUAL "SHARED_LIBRARY")
                    set(cmd ${cmd} -i $<TARGET_FILE:${dep}>)
                    list(APPEND SOLIBS "${dep}")
                endif()
            endif()
        endforeach()
    endforeach()

    set(cmd ${cmd} -l $<TARGET_FILE_NAME:${A_APP}>@/usr/bin/mepa-demo-edsx)
#    add_custom_command(
#        TARGET cli POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                ${CMAKE_SOURCE_DIR}/mesa/demo/scripts
#                ${CMAKE_CURRENT_BINARY_DIR}/.)
    # Add overlays
    set(cmd ${cmd} -i ${CMAKE_CURRENT_SOURCE_DIR}/overlay)



    # Make sure the 'init' program is installed, even though it is not always needed
    list(APPEND EXECUTABLES init)

    if (IMG_${A_NAME}_${A_TYPE})
        if (${A_TYPE} STREQUAL "fit")
            #file(COPY ${CMAKE_SOURCE_DIR}/mesa/demo/mepa_scripts DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.)
            add_custom_command(
                OUTPUT ${A_NAME}.itb
                DEPENDS ${KERNEL} ${EXECUTABLES} ${SOLIBS} ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb 
                COMMAND ${CMAKE_COMMAND} -E env "PATH=\"${RELOCATED_HOST_DIR}/bin:${RELOCATED_HOST_DIR}/sbin:$ENV{PATH}\"" ${cmd} -t fit
                COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
            )
            add_custom_target(${A_NAME}_itb ALL DEPENDS ${A_NAME}.itb)

        elseif (${A_TYPE} STREQUAL "ext4")
            # When building ext4 image, also build a fit as we need this for ramloading
            add_custom_command(
                OUTPUT ${A_NAME}.itb
                DEPENDS ${KERNEL} ${EXECUTABLES} ${SOLIBS} ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
                COMMAND ${CMAKE_COMMAND} -E env "PATH=\"${RELOCATED_HOST_DIR}/bin:${RELOCATED_HOST_DIR}/sbin:$ENV{PATH}\"" ${cmd} -t fit
                COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
            )
            add_custom_target(${A_NAME}_itb ALL DEPENDS ${A_NAME}.itb)

            add_custom_command(
                OUTPUT ${A_NAME}.ext4.gz ${A_NAME}.ubifs
                DEPENDS ${KERNEL} ${EXECUTABLES} ${SOLIBS} ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
                COMMAND ${CMAKE_COMMAND} -E env "PATH=\"${RELOCATED_HOST_DIR}/bin:${RELOCATED_HOST_DIR}/sbin:$ENV{PATH}\"" ${cmd} -t ext4
                COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
            )
            add_custom_target(${A_NAME}_ext4 ALL DEPENDS ${A_NAME}.ext4.gz ${A_NAME}.ubifs)

        elseif (${A_TYPE} STREQUAL "mfi")
            add_custom_command(
                OUTPUT ${A_NAME}.mfi
                DEPENDS ${KERNEL} ${EXECUTABLES} ${SOLIBS} ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
                COMMAND ${CMAKE_COMMAND} -E env "PATH=\"${RELOCATED_HOST_DIR}/bin:${RELOCATED_HOST_DIR}/sbin:$ENV{PATH}\"" ${cmd} -t mfi
                COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/imggen.rb
            )
            add_custom_target(${A_NAME}_mfi ALL DEPENDS ${A_NAME}.mfi)

        endif()
    endif()
endmacro()

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    APP_TARGET(NAME app_vsc7558_edsx         MESA vsc7558          MEBA meba_edsx          APP_TARGET_DEFINES -DMEPA_HAS_LAN80XX -DMEPA_HAS_VTSS -DMEPA_DEMO_EDSx)
    IMG_TARGET(NAME arm64_vsc7558_edsx       TYPE ext4             APP app_vsc7558_edsx    MACHINE fireant)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
    APP_TARGET(NAME app_lan966x            MESA lan966x            MEBA meba_eds2          APP_TARGET_DEFINES -DMEPA_DEMO_EDS2)
    IMG_TARGET(NAME armv7_lan966x          TYPE ext4               APP app_lan966x         MACHINE lan966x)
endif()

