# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

macro(MEPA_DRV)
    set(oneValueArgs   LIB_NAME)
    set(multiValueArgs SRCS DEFS INCL_PUB INCL_PRI)

    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    option(BUILD_${A_LIB_NAME} "Build the STATIC MEPA layer for ${A_LIB_NAME}" OFF)
    mark_as_advanced(${A_LIB_NAME})

    list(JOIN A_INCL_PUB $<SEMICOLON> public_includes)
    list(JOIN A_INCL_PRI $<SEMICOLON> private_includes)

    add_library(${A_LIB_NAME} STATIC ${A_SRCS})
    target_include_directories(${A_LIB_NAME} 
        PUBLIC 
            $<BUILD_INTERFACE:${public_includes}>
        PRIVATE
            $<BUILD_INTERFACE:${private_includes}>
    )

    if (${MEPA_OPSYS_VELOCITYSP})
        list(APPEND A_DEFS -DMEPA_OPSYS_VELOCITYSP=1)
    endif()
    target_compile_definitions(${A_LIB_NAME} PUBLIC ${A_DEFS})

    if (${BUILD_ALL})
        set(BUILD_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
    endif()

    if (${BUILD_MEPA_ALL})
        set(BUILD_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
    endif()

    if (${BUILD_${A_LIB_NAME}})
        message(STATUS "Build ${A_LIB_NAME} including ${A_MEPA_DEFINES}")
        set_target_properties(${A_LIB_NAME} PROPERTIES EXCLUDE_FROM_ALL FALSE)
        install_targets(TARGETS ${A_LIB_NAME})
    else()
        set_target_properties(${A_LIB_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endmacro()

macro(mepa_merge_static_libs)
    set(oneValueArgs   TARGET)
    set(multiValueArgs LIBRARIES)

    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_library(${A_TARGET} STATIC)
    foreach(dep ${A_LIBRARIES})
        target_sources(${A_TARGET} PRIVATE $<TARGET_OBJECTS:${dep}>)
        set_target_properties(${A_TARGET}
            PROPERTIES
                INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${dep},INCLUDE_DIRECTORIES>
                COMPILE_DEFINITIONS $<TARGET_PROPERTY:${dep},COMPILE_DEFINITIONS>
        )
    endforeach()
endmacro(mepa_merge_static_libs)

macro(MEPA_LIB)
    set(options        ADVANCED)
    set(oneValueArgs   LIB_NAME)
    set(multiValueArgs DEFS DRVS)
    cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if (A_ADVANCED)
        option(BUILD_${A_LIB_NAME} "Build ${A_LIB_NAME} with ${A_DRVS}" OFF)
        mark_as_advanced(BUILD_${A_LIB_NAME})
    else()
        option(BUILD_${A_LIB_NAME} "Build ${A_LIB_NAME} with ${A_DRVS}" ON)
    endif()

    set(lib_common ${A_LIB_NAME}_common)
    add_library(${lib_common} STATIC EXCLUDE_FROM_ALL ${MEPA_SOURCE_DIR}/common/src/phy.c)
    if (${MEPA_OPSYS_VELOCITYSP})
        list(APPEND A_DEFS -DMEPA_OPSYS_VELOCITYSP=1)
    endif()
    target_link_libraries(${lib_common} PRIVATE ${A_DRVS})
    target_compile_definitions(${lib_common} PRIVATE ${A_DEFS})
    target_include_directories(${lib_common}
                               PUBLIC 
                                  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/me/include$<SEMICOLON>${MEPA_SOURCE_DIR}/include>
                                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                               PRIVATE $<BUILD_INTERFACE:${MEPA_SOURCE_DIR}/common/src>)

    mepa_merge_static_libs(TARGET    ${A_LIB_NAME}
                           LIBRARIES ${lib_common} ${A_DRVS})

    if (${BUILD_ALL})
        set(BUILD_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
    endif()

    if (${BUILD_MEPA_ALL})
        set(BUILD_${A_LIB_NAME} ON CACHE BOOL "" FORCE)
    endif()

    if (${BUILD_${A_LIB_NAME}})
        message(STATUS "Build ${A_LIB_NAME} including ${A_DRVS}")
        set_target_properties(${A_LIB_NAME} PROPERTIES EXCLUDE_FROM_ALL FALSE)
    else()
        set_target_properties(${A_LIB_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()

    install_targets(TARGETS ${A_LIB_NAME})
endmacro()


