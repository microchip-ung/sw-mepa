# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

include(./.cmake/flags.cmake)

# Only include the following if we are the top level project
if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
  include(./.cmake/mkid.cmake)
  include(./.cmake/tags.cmake)
endif()

macro(install_targets)
    set(options        "")
    set(oneValueArgs   "")
    set(multiValueArgs TARGETS)

    cmake_parse_arguments(_args "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(_tgt ${_args_TARGETS})
      set(_tgt_path $<PATH:RELATIVE_PATH,$<TARGET_FILE_DIR:${_tgt}>,${CMAKE_BINARY_DIR}>)
      install(
          TARGETS ${_tgt}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${_tgt_path}
          PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mepa
          INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mepa
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${_tgt_path}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${_tgt_path}
      )
    endforeach()
endmacro()