# Diagnostic fixtures are linked only for explicitly requested developer builds.
include_guard(GLOBAL)
option(ENTISGLS_BUILD_DIAGNOSTICS "Build original-game probes and runtime diagnostic entry points" OFF)

function(entis_attach_cotopha_diagnostics app_target)
    if(NOT ENTISGLS_BUILD_DIAGNOSTICS)
        return()
    endif()
    if(NOT TARGET cotopha_diagnostics)
        get_filename_component(_entis_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
        add_library(cotopha_diagnostics STATIC
            "${_entis_root}/tests/probes/cotopha/legacy_atomic_save_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_audio_player_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_context_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_core_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_emote_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_file_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_heap_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_image_export_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_input_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_media_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_movie_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_movie_window_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_native_binding_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_primary_context_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_resource_state_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_setup_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_sprite_dynamic_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_sprite_state_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_thread_state_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_volume_envelope_probe.cpp"
            "${_entis_root}/tests/probes/cotopha/legacy_window_probe.cpp"
        )
        set_target_properties(cotopha_diagnostics PROPERTIES POSITION_INDEPENDENT_CODE ON
            CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
        target_compile_options(cotopha_diagnostics PRIVATE -fno-delete-null-pointer-checks
            -Wno-tautological-undefined-compare -Wno-invalid-source-encoding
            -Wno-invalid-offsetof -Wno-deprecated-declarations)
        target_link_libraries(cotopha_diagnostics PUBLIC legacy_objects legacy_foundation)
        if(STUDYSTEADY_SDL3)
            target_link_libraries(cotopha_diagnostics PUBLIC gls4_sdl motion_apk_runtime)
        endif()
    endif()
    target_compile_definitions(${app_target} PRIVATE ENTISGLS_BUILD_DIAGNOSTICS=1)
    target_link_libraries(${app_target} PRIVATE cotopha_diagnostics)
endfunction()
