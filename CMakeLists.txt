cmake_minimum_required(VERSION 3.21)

project(samp-plugin-sdk LANGUAGES C)

include(GNUInstallDirs)

if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "samp-plugin-sdk supports Windows and Linux only")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "samp-plugin-sdk is x86-only. "
        "Use an x86 compiler/toolchain (or one of the provided CMake presets).")
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|Clang|MSVC)$")
    message(FATAL_ERROR
        "Unsupported C compiler '${CMAKE_C_COMPILER_ID}'. "
        "Supported compilers are GCC, Clang, and MSVC.")
endif()

add_library(samp-plugin-sdk STATIC
    amxplugin.c
)
add_library(samp::plugin-sdk ALIAS samp-plugin-sdk)

set_target_properties(samp-plugin-sdk PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE YES
    EXPORT_NAME plugin-sdk
)

target_include_directories(samp-plugin-sdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SampPlugin.cmake")

option(
    SAMP_PLUGIN_SDK_BUILD_EXAMPLE
    "Build the example SA-MP plugin"
    ${PROJECT_IS_TOP_LEVEL}
)

if(SAMP_PLUGIN_SDK_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

option(
    SAMP_PLUGIN_SDK_INSTALL
    "Generate install rules for samp-plugin-sdk"
    ${PROJECT_IS_TOP_LEVEL}
)

if(SAMP_PLUGIN_SDK_INSTALL)
    install(
        TARGETS samp-plugin-sdk
        EXPORT samp-plugin-sdk-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(
        FILES plugincommon.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        FILES
            amx/amx.h
            amx/amx2.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/amx
    )

    set(SAMP_PLUGIN_SDK_CMAKE_DIR
        "${CMAKE_INSTALL_LIBDIR}/cmake/samp-plugin-sdk"
    )

    install(
        EXPORT samp-plugin-sdk-targets
        FILE samp-plugin-sdk-targets.cmake
        NAMESPACE samp::
        DESTINATION ${SAMP_PLUGIN_SDK_CMAKE_DIR}
    )

    install(
        FILES
            cmake/SampPlugin.cmake
            cmake/samp-plugin-sdk-config.cmake
        DESTINATION ${SAMP_PLUGIN_SDK_CMAKE_DIR}
    )
endif()

