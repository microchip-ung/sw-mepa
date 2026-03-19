# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

project(mepa_api)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(CMakeParseArguments)
include(.cmake/common.cmake)

option(BUILD_ALL         "Enable all" OFF)
option(BUILD_STATIC_LIBS "Build static versions of the libraries" ON)
option(BUILD_SHARED_LIBS "Build shared versions of the libraries" ON)
option(BUILD_MESA_ALL    "Build the MESA libraries" OFF)
option(BUILD_DOXYGEN     "Build doxygen" OFF)
option(MESA_PRE_AG       "Use pre-generated mesa-ag layer if exist" ON)
option(BUILD_MEPA_DEMO   "Build the MEPA demo" ON)
option(MESA_OPSYS_LINUX  "Build the MESA for Linux" ON)
option(MESA_OPSYS_VELOCITYSP "Build the MESA for VelocitySP" OFF)
option(MESA_WRAP         "Generate and build the MESA Wrapper" ON)
option(MESA_USE_INLINE_UTILS "Create an object library instead of static/shared library" ON)

if (NOT MESA_USE_INLINE_UTILS)
    message(FATAL_ERROR "MESA_USE_INLINE_UTILS must be ON. Shared or static lm_utils is not supported.")
endif ()

mark_as_advanced(BUILD_MEPA_DEMO MESA_PRE_AG BUILD_DOXYGEN)

include(.cmake/doxygen.cmake)

add_subdirectory(mepa)

include_directories(me/include)
include_directories(sw-mesa/mesa/include)
include_directories(sw-mesa/utils/include)
include_directories(board-configs/include)
include_directories(mepa/include)

set (OS_CNT 0)

#LM Utils for sw-mesa
set(lm_utils_src sw-mesa/utils/src/lm_utils.c sw-mesa/utils/src/lm_utils_trace_conf.c)

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv8")
        include_directories(sw-mesa/utils/src/arm-optimized-routines/string/aarch64)
        include_directories(sw-mesa/utils/src/arm-optimized-routines/string/include)
        list(APPEND lm_utils_src sw-mesa/utils/src/arm-optimized-routines/string/aarch64/memcmp.S)
        list(APPEND lm_utils_src sw-mesa/utils/src/arm-optimized-routines/string/aarch64/memcpy.S)
        list(APPEND lm_utils_src sw-mesa/utils/src/arm-optimized-routines/string/aarch64/memset.S)

    elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
        include_directories(sw-mesa/utils/src/arm-optimized-routines/string/arm)
        include_directories(sw-mesa/utils/src/arm-optimized-routines/string/include)
        list(APPEND lm_utils_src sw-mesa/utils/src/arm-optimized-routines/string/arm/memcpy.S)
        list(APPEND lm_utils_src sw-mesa/utils/src/arm-optimized-routines/string/arm/memset.S)
    endif ()
endif ()

list(APPEND lm_utils_src sw-mesa/utils/src/lm_utils_memcmp.c)
list(APPEND lm_utils_src sw-mesa/utils/src/lm_utils_memcpy.c)
list(APPEND lm_utils_src sw-mesa/utils/src/lm_utils_memmove.c)
list(APPEND lm_utils_src sw-mesa/utils/src/lm_utils_memset.c)

if (${MESA_USE_INLINE_UTILS})
    add_library(lm_utils OBJECT ${lm_utils_src})
    target_compile_options(lm_utils PRIVATE -fPIC)
else ()
    add_library(lm_utils ${lm_utils_src})
endif ()

target_include_directories(lm_utils PRIVATE sw-mesa/utils/include/include)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(lm_utils PRIVATE -DLMU_OS_LINUX)
endif ()

# Detect wether the compiler either support __VA_OPT__ or the GNU extension for __VA_ARGS__
# (this test is skipped for Clang and GCC 13)
if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang" AND
   NOT (CMAKE_C_COMPILER_ID MATCHES "GNU" AND  CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 13 AND CMAKE_C_COMPILER_VERSION VERSION_LESS 14))

include(CheckCSourceCompiles)
include(CMakePushCheckState)

check_c_source_compiles([[
  #define FOO(...) __VA_OPT__(int) __VA_ARGS__
  int main(void) {
      int x = FOO() 4;
      FOO(y = 2);
      return x + y;
  }
]] HAS___VA_OPT__)

if(HAS___VA_OPT__)
    add_compile_definitions(LMU_PP_USE___VA_OPT__)
else()
    cmake_push_check_state(RESET)

    try_compile(HAS__VA_ARG__GNU_EXT
        SOURCE_FROM_CONTENT test_va_arg_gnu_ext.c [[
            #include <assert.h>
            #define SEQ() 2, 1, 0
            #define ARGN(_1, _2, N, ...) N
            #define COUNT_IMPL(_, ...) ARGN(__VA_ARGS__)
            #define COUNT(...) COUNT_IMPL(0, ##__VA_ARGS__, SEQ())
            int main(void) {
            _Static_assert(0 == COUNT());
            _Static_assert(1 == COUNT(foo));
            _Static_assert(2 == COUNT(foo, bar));
            return 0;
            }
        ]]
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        OUTPUT_VARIABLE OUTPUT
    )

    cmake_pop_check_state()

    if(NOT HAS__VA_ARG__GNU_EXT)
        message(FATAL_ERROR "${CMAKE_C_COMPILER} does not support __VA_ARG__ GNU extension.")
    else()
        set(CMAKE_C_EXTENSIONS ON)
    endif()
endif()

endif()

if (${MESA_OPSYS_LINUX})
    add_definitions(-DLMU_BOOL_IS_UINT8)
endif()

if (${MESA_OPSYS_LINUX})
    MATH(EXPR OS_CNT "${OS_CNT} + 1")
endif ()

if (${MESA_OPSYS_VELOCITYSP})
    MATH(EXPR OS_CNT "${OS_CNT} + 1")
endif ()

if (NOT ${OS_CNT} EQUAL 1)
    message(FATAL_ERROR "Enable one and only one OS layer")
endif ()

# board-configs is only supported on Linux
if (${MESA_OPSYS_LINUX})
	add_subdirectory(board-configs)
endif ()

#find_program(CCACHE_FOUND ccache)
#if(CCACHE_FOUND)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
#endif(CCACHE_FOUND)

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckCSourceCompiles)

if (${MESA_OPSYS_LINUX})
    CHECK_INCLUDE_FILES("endian.h" VTSS_HAS_ENDIAN_H)
    IF(NOT VTSS_HAS_ENDIAN_H)
        message(FATAL_ERROR "endian.h was not found or did not compile. See CMakeFiles/CMakeError.log for more details.")
    ENDIF()

    CHECK_INCLUDE_FILES("asm/byteorder.h" VTSS_HAS_ASM_BYTEORDER_H)
    IF(NOT VTSS_HAS_ASM_BYTEORDER_H)
        message(FATAL_ERROR "asm/byteorder.h was not found or did not compile. See CMakeFiles/CMakeError.log for more details.")
    ENDIF()
endif ()

file(GLOB_RECURSE API_UNI_SRC  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "sw-mesa/base/*.c")
file(GLOB_RECURSE API_UNI_HDR  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/include" "sw-mesa/include/**.h")
file(GLOB_RECURSE API_UNI_HDR_ABS "include/**.h")

if (${MESA_WRAP})
    file(GLOB_RECURSE API_MESA_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "sw-mesa/mesa/src/**.c")
    file(GLOB_RECURSE API_MESA_HDR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/mesa/include" "sw-mesa/mesa/include/**.h")
    file(GLOB_RECURSE API_MEBA_HDR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/board-configs/include" "board-configs/include/**.h")
endif ()

file(GLOB_RECURSE API_MEPA_HDR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/mepa/include" "mepa/include/**.h")
file(GLOB_RECURSE API_MEPA_VTSS_HDR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/mepa/vtss/include" "mepa/vtss/include/*.h")
file(GLOB_RECURSE API_ME_HDR RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/me/include" "sw-mesa/me/include/**.h")

list(REMOVE_ITEM API_MESA_SRC sw-mesa/mesa/src/capability_dumper.c)

### MESA-PROCESSING START #####################################################################################################################################
if (${MESA_WRAP})
if (EXISTS mesa/ag/mesa.c AND EXISTS mesa/ag/mesa.h AND ${MESA_PRE_AG})

message(STATUS "Using pre-generated mesa-ag layer")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/masa/ag/)
list(APPEND API_MESA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/masa/ag/mesa.c)
list(APPEND API_MESA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/masa/ag/mesa.h)

else()

set(API_MESA_CACHE_IN)
set(API_MESA_CACHE_OUT)

set(API_ME_CACHE_IN)
list(APPEND API_ME_CACHE_IN me/include/microchip/ethernet/common.h)

# List of header files to exclude from the mesa-hdr processing
set(MESA_CACHE_DENY_LIST include/vtss_os_linux.h
                          include/vtss_os_ecos.h
                          include/vtss_ae_api.h
                          mesa/include/microchip/ethernet/hdr_end.h
                          mesa/include/microchip/ethernet/hdr_start.h
                          mesa/include/microchip/ethernet/switch/api/port_list.h)

# Macro to produce the mesa processing cache
macro(MESA_CACHE)
    list(FIND MESA_CACHE_DENY_LIST ${ARGV0} BL)
    if(-1 EQUAL ${BL})
        #message(STATUS "mesa_cache: ${ARGV0}")
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/${ARGV0}.cache
                           COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/mesa/scripts/ag.rb -b ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa -o ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag --cache-only ${ARGV0}
                           DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/${ARGV0}
                           COMMENT "AG: Parsing/caching ${ARGV0}")
        list(APPEND API_MESA_CACHE_IN  ${ARGV0})
        list(APPEND API_MESA_CACHE_OUT ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/${ARGV0}.cache)
    else()
        #message(STATUS "skipping: ${ARGV0}")
    endif()
endmacro(MESA_CACHE)

FOREACH(f ${API_UNI_HDR})
    # Add header to mesa processing
    MESA_CACHE("include/${f}")
ENDFOREACH()

FOREACH(f ${API_MESA_HDR})
    # Install all mesa headers
    configure_file("sw-mesa/mesa/include/${f}" "${CMAKE_CURRENT_BINARY_DIR}/include_common/${f}" COPYONLY)
    # Add header to mesa processing
    MESA_CACHE("mesa/include/${f}")
ENDFOREACH()

FOREACH(f ${API_MEBA_HDR})
    configure_file("board-configs/include/${f}" "${CMAKE_CURRENT_BINARY_DIR}/include_common/${f}" COPYONLY)
ENDFOREACH()

FOREACH(f ${API_MEPA_HDR})
    # Add all mepa headers
    configure_file("mepa/include/${f}" "${CMAKE_CURRENT_BINARY_DIR}/include_common/${f}" COPYONLY)
ENDFOREACH()

FOREACH(f ${API_MEPA_VTSS_HDR})
    configure_file("mepa/vtss/include/${f}" "${CMAKE_CURRENT_BINARY_DIR}/include_common/${f}" COPYONLY)
ENDFOREACH()

FOREACH(f ${API_ME_HDR})
    # Add all me headers
    configure_file("sw-mesa/me/include/${f}" "${CMAKE_CURRENT_BINARY_DIR}/include_common/${f}" COPYONLY)
ENDFOREACH()

# Target to drive the mesa-cache processing
add_custom_target(mesa_cache DEPENDS ${API_MESA_CACHE_OUT})

# Create the auto generated mesa header/src (using the earlier created cache).
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.h ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.c
                   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/mesa/scripts/ag.rb -b ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa -o ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag ${API_MESA_CACHE_IN} ${API_ME_CACHE_IN}
                   DEPENDS mesa_cache ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/mesa/scripts/ag.rb
                   COMMENT "Generate mesa.h/mesa.c")
add_custom_target(mesa_src DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.h ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.c)
list(APPEND API_MESA_SRC ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.c)
list(APPEND API_MESA_SRC ${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/mesa.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/mesa-ag/)
### MESA-PROCESSING END #######################################################################################################################################
endif()
endif()

include_directories(sw-mesa/include)

################################################################################
# Find all .c and .h files recursively in the repo
file(GLOB_RECURSE ALL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/mepa/microchip/*.c"
    "${CMAKE_SOURCE_DIR}/mepa/microchip/*.h"
    "${CMAKE_SOURCE_DIR}/mepa_demo/mepa_apps/*.c"
    "${CMAKE_SOURCE_DIR}/mepa_demo/mepa_apps/*.h"
    "${CMAKE_SOURCE_DIR}/board-configs/*.c"
    "${CMAKE_SOURCE_DIR}/board-configs/*.h"
)

# List of files to exclude from formatting
list(APPEND EXCLUDED_FILES
    "${CMAKE_SOURCE_DIR}/mepa/microchip/lan80xx/src/lan80xx_serdes_config.h"
    "${CMAKE_SOURCE_DIR}/mepa/microchip/lan80xx/src/sha256/platform_util.h"
    "${CMAKE_SOURCE_DIR}/mepa/microchip/lan80xx/src/sha256/sha256.c"
    "${CMAKE_SOURCE_DIR}/mepa/microchip/lan80xx/src/sha256/sha256.h"
    "${CMAKE_SOURCE_DIR}/mepa/microchip/lan80xx/src/sha256/sha256_config.h"
)

# Remove excluded files from ALL_SOURCE_FILES list
foreach(EXCLUDED_FILE ${EXCLUDED_FILES})
    message(STATUS "Excluding file: ${EXCLUDED_FILE}")  # Print the excluded file
    list(REMOVE_ITEM ALL_SOURCE_FILES ${EXCLUDED_FILE})
endforeach()

# Add a format target using your custom AStyle config
add_custom_target(format
    COMMAND astyle --options=${CMAKE_SOURCE_DIR}/.cmake/astyle ${ALL_SOURCE_FILES}
    COMMENT "Formatting all .c and .h files using AStyle with .astylerc"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


set(LIST_OF_API_TARGETS)

if (${MESA_OPSYS_LINUX})
    set(GLOBAL_DEFS -DVTSS_OPSYS_LINUX -DVTSS_OPT_SYMREG=1)
elseif (${MESA_OPSYS_VELOCITYSP})
	set(GLOBAL_DEFS -DVTSS_OPSYS_VELOCITYSP)
endif ()

macro(API_TARGET)
  set(oneValueArgs LIB CHIP)
  set(multiValueArgs DEFS)

  cmake_parse_arguments(A "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if (${BUILD_SHARED_LIBS})
	  option(${A_LIB}        "Build the API for ${A_CHIP} (${A_LIB})" OFF)
  endif()
  option(${A_LIB}_static "Build the static API for ${A_CHIP} (${A_LIB})" OFF)
  if (${BUILD_MESA_ALL} OR ${BUILD_ALL})
      if (${BUILD_SHARED_LIBS})
          set(${A_LIB} ON CACHE BOOL "" FORCE)
      endif()

      if (${BUILD_STATIC_LIBS})
          set(${A_LIB}_static ON CACHE BOOL "" FORCE)
      endif()
  endif()

  set(DEFS ${GLOBAL_DEFS} -DVTSS_CHIP_${A_CHIP} ${A_DEFS})

  if (${BUILD_SHARED_LIBS})
      if (${MESA_USE_INLINE_UTILS})
          add_library(${A_LIB} SHARED $<TARGET_OBJECTS:lm_utils> ${API_UNI_SRC} ${API_MESA_SRC})
      else ()
          add_library(${A_LIB} SHARED ${API_UNI_SRC} ${API_MESA_SRC})
          target_link_libraries(${A_LIB} lm_utils)
      endif ()

      target_compile_options(${A_LIB} PUBLIC ${DEFS}
                                      PRIVATE -Wno-logical-op)

      if (${A_LIB})
          message(STATUS "Adding ${A_LIB} (${A_CHIP}) MESA=${has_mesa} Defines: [${DEFS}]")
          set_target_properties(${A_LIB} PROPERTIES EXCLUDE_FROM_ALL FALSE)
      else()
          set_target_properties(${A_LIB} PROPERTIES EXCLUDE_FROM_ALL TRUE)
      endif()
  endif()

  if (${MESA_USE_INLINE_UTILS})
      add_library(${A_LIB}_static STATIC $<TARGET_OBJECTS:lm_utils> ${API_UNI_SRC} ${API_MESA_SRC})
  else ()
      add_library(${A_LIB}_static STATIC ${API_UNI_SRC} ${API_MESA_SRC})
      target_link_libraries(${A_LIB}_static lm_utils)
  endif ()

  target_compile_options(${A_LIB}_static PUBLIC ${DEFS}
	                                 PRIVATE -Wno-format-security -Wno-sign-compare -Wno-type-limits -Wno-implicit-fallthrough -Wno-array-parameter -Wno-enum-conversion -Wno-logical-op)
  
  if (${A_LIB}_static)
      message(STATUS "Adding ${A_LIB}_static (${A_CHIP}) MESA=${has_mesa} Defines: [${DEFS}]")
      set_target_properties(${A_LIB}_static PROPERTIES EXCLUDE_FROM_ALL FALSE)
  else()
      set_target_properties(${A_LIB}_static PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endif()

  add_custom_command(
      OUTPUT "include_${A_LIB}/vtss/api/options.h"
      COMMAND mkdir -p include_${A_LIB}/vtss/api
      COMMAND gcc -dM -E ${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/include/vtss/api/options.h ${DEFS} | grep VTSS > include_${A_LIB}/vtss/api/options.h
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sw-mesa/include/vtss/api/options.h"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Preprocessing options.h for ${A_LIB}"
  )

  list(APPEND LIST_OF_API_TARGETS ${A_LIB})
endmacro(API_TARGET)

API_TARGET(LIB vsc7558               CHIP 7558TSN       DEFS -DVTSS_OPT_PORT_COUNT=57)
API_TARGET(LIB lan966x               CHIP 966X)

option(BUILD_MESA_CAPABILITY_DUMPER "Build all the mesa capability dumper function" OFF)
mark_as_advanced(BUILD_MESA_CAPABILITY_DUMPER)
if (${BUILD_MESA_CAPABILITY_DUMPER})
    add_executable(capability_dumper mesa/src/capability_dumper.c)
    target_link_libraries(capability_dumper dl)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sw-mera/CMakeLists.txt)
    set(HAS_RTE ON)
    add_subdirectory(sw-mera)
else()
    set(HAS_RTE OFF)
endif()

if (${BUILD_MEPA_DEMO})
    find_library(JSON_LIB json-c)
    mark_as_advanced(JSON_LIB)
    if (JSON_LIB)
        add_subdirectory(mepa_demo)
    endif()
endif()

