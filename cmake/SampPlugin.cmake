include_guard(GLOBAL)

function(samp_add_plugin target)
    set(options AMX_NATIVES PROCESS_TICK)
    set(multi_value_args SOURCES)

    cmake_parse_arguments(
        SAMP_PLUGIN
        "${options}"
        ""
        "${multi_value_args}"
        ${ARGN}
    )

    if(SAMP_PLUGIN_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "samp_add_plugin(${target}): unknown arguments: "
            "${SAMP_PLUGIN_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT SAMP_PLUGIN_SOURCES)
        message(FATAL_ERROR
            "samp_add_plugin(${target}): SOURCES must not be empty")
    endif()

    if(TARGET "${target}")
        message(FATAL_ERROR
            "samp_add_plugin(${target}): target already exists")
    endif()

    if(NOT TARGET samp::plugin-sdk)
        message(FATAL_ERROR
            "samp_add_plugin(${target}): samp::plugin-sdk is not available")
    endif()

    add_library("${target}" MODULE ${SAMP_PLUGIN_SOURCES})

    target_link_libraries("${target}"
        PRIVATE
            samp::plugin-sdk
    )

    set_target_properties("${target}" PROPERTIES
        PREFIX ""
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )

    if(WIN32)
        set(exports
            "EXPORTS\n"
            "    Supports\n"
            "    Load\n"
            "    Unload\n"
        )

        if(SAMP_PLUGIN_AMX_NATIVES)
            string(APPEND exports
                "    AmxLoad\n"
                "    AmxUnload\n"
            )
        endif()

        if(SAMP_PLUGIN_PROCESS_TICK)
            string(APPEND exports
                "    ProcessTick\n"
            )
        endif()

        set(def_file
            "${CMAKE_CURRENT_BINARY_DIR}/${target}.exports.def"
        )

        file(CONFIGURE
            OUTPUT "${def_file}"
            CONTENT "${exports}"
            @ONLY
        )

        target_sources("${target}" PRIVATE "${def_file}")
    endif()
endfunction()

