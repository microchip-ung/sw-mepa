# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

project(meba)

cmake_minimum_required(VERSION 3.15)

include_directories(include)
include_directories(./src)
include_directories(./src/drivers)
include_directories(../mepa/vtss/include)
include_directories(../mepa/common/include)
include_directories(../me/include)
include_directories(../mepa_demo)
include_directories(../mepa/microchip/lan80xx/include/microchip)

option(BUILD_BOARD_CFG_ALL "Build all Board Config layers" OFF)
mark_as_advanced(BUILD_BOARD_CFG_ALL)

macro(merge_static_libs)
    set(oneValueArgs   TARGET FILENAME)
    set(multiValueArgs LIBRARIES)

    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(WD ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.tmp)

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar.in "CREATE ${A_FILENAME}\n" )
    foreach(e ${A_LIBRARIES})
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar.in
            "ADDLIB $<TARGET_FILE:${e}>\n")
    endforeach()
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar.in "SAVE\n")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar.in "END\n")
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar.in)

    add_custom_command(
        COMMAND ${CMAKE_AR} -Ms < ${CMAKE_CURRENT_BINARY_DIR}/${A_TARGET}.ar
        OUTPUT  ${A_FILENAME}
        DEPENDS ${A_LIBRARIES}
        COMMENT "Bundling ${A_TARGET}"
        VERBATIM
    )

    add_custom_target(${A_TARGET}_target DEPENDS ${A_FILENAME})

    add_library(${A_TARGET} STATIC IMPORTED GLOBAL)
    add_dependencies(${A_TARGET} ${A_TARGET}_target)
    set_target_properties(${A_TARGET} PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${A_FILENAME}
    )
endmacro(merge_static_libs)

macro(MEBA_LIB)
    set(oneValueArgs   LIB_NAME MEBA_SRC_FOLDER )
    set(multiValueArgs STATIC_DEPENDENCIES DYNAMIC_DEPENDENCIES MEBA_DEFINES)

    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    option(BUILD_MEBA_${A_LIB_NAME}        "Build the MEBA layer for ${A_LIB_NAME}" OFF)
    option(BUILD_MEBA_STATIC_${A_LIB_NAME} "Build the STATIC MEBA layer for ${A_LIB_NAME}" OFF)

    #mark_as_advanced(BUILD_MEBA_${A_LIB_NAME} BUILD_MEBA_${A_LIB_NAME}_static)
    FILE(GLOB c_files
         RELATIVE
         "${CMAKE_CURRENT_SOURCE_DIR}"
         "${CMAKE_CURRENT_SOURCE_DIR}/src/${A_MEBA_SRC_FOLDER}/*.c"
         "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
         "${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/*.c")

    add_library(meba_${A_LIB_NAME} SHARED ${c_files})
    target_compile_definitions(meba_${A_LIB_NAME} PRIVATE ${A_MEBA_DEFINES})
    target_link_libraries(meba_${A_LIB_NAME} ${A_DYNAMIC_DEPENDENCIES})

    add_library(meba_only_${A_LIB_NAME} STATIC ${c_files} )
    target_compile_definitions(meba_only_${A_LIB_NAME} PRIVATE ${A_MEBA_DEFINES})

    merge_static_libs(TARGET meba_${A_LIB_NAME}_static FILENAME libmeba_${A_LIB_NAME}.a
                      LIBRARIES meba_only_${A_LIB_NAME} ${A_STATIC_DEPENDENCIES})

    if (${BUILD_ALL} OR ${BUILD_BOARD_CFG_ALL})
        set(BUILD_MEBA_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
        set(BUILD_MEBA_STATIC_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
    endif()

    if (${BUILD_MEBA_${A_LIB_NAME}})
        message(STATUS "Build SHARED ${A_LIB_NAME} including ${A_MEBA_DEFINES}")
        set_target_properties(meba_${A_LIB_NAME}        PROPERTIES EXCLUDE_FROM_ALL FALSE)
    else()
        set_target_properties(meba_${A_LIB_NAME}        PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()

    if (${BUILD_MEBA_STATIC_${A_LIB_NAME}})
        message(STATUS "Build STATIC ${A_LIB_NAME} including ${A_MEBA_DEFINES}")
        set_target_properties(meba_${A_LIB_NAME}_static PROPERTIES EXCLUDE_FROM_ALL FALSE)
        set_target_properties(meba_${A_LIB_NAME}_static_target PROPERTIES EXCLUDE_FROM_ALL FALSE)
        set_target_properties(meba_only_${A_LIB_NAME} PROPERTIES EXCLUDE_FROM_ALL FALSE)
    else()
        set_target_properties(meba_${A_LIB_NAME}_static PROPERTIES EXCLUDE_FROM_ALL TRUE)
        set_target_properties(meba_${A_LIB_NAME}_static_target PROPERTIES EXCLUDE_FROM_ALL TRUE)
        set_target_properties(meba_only_${A_LIB_NAME}  PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endmacro()

MEBA_LIB(LIB_NAME sparx5 MEBA_SRC_FOLDER sparx5
         STATIC_DEPENDENCIES    mepa_sparx5
	 MEBA_DEFINES           -DMEPA_DEMO_EDSx)

MEBA_LIB(LIB_NAME sparx5_macsec MEBA_SRC_FOLDER sparx5
         STATIC_DEPENDENCIES    mepa_sparx5_macsec
	 MEBA_DEFINES           -DMEPA_DEMO_EDSx)

MEBA_LIB(LIB_NAME eds2 MEBA_SRC_FOLDER eds2
         STATIC_DEPENDENCIES    mepa_eds2
	 MEBA_DEFINES -DMEPA_DEMO_EDS2)

