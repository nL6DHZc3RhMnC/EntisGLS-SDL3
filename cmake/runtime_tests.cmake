# Synthetic tests remain available without original game data or diagnostic probes.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(sdl_system_test EXCLUDE_FROM_ALL tests/integration/platform/system_test.cpp)
    target_link_libraries(sdl_system_test PRIVATE gls4_sdl)
endif()

if(NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(game_save_directory_test EXCLUDE_FROM_ALL tests/unit/launcher/save_directory_test.cpp
        native/io/save_directory.cpp native/runtime/cotopha_port/legacy_atomic_path.cpp)
    target_compile_definitions(game_save_directory_test PRIVATE STUDYSTEADY_PLATFORM_SDL3=1)
    target_link_libraries(game_save_directory_test PRIVATE entis_game_files)
    add_executable(psb_key_resolver_test EXCLUDE_FROM_ALL tests/unit/launcher/psb_key_resolver_test.cpp)
    target_link_libraries(psb_key_resolver_test PRIVATE entis_psb_keys)
    add_executable(psb_key_settings_test EXCLUDE_FROM_ALL tests/unit/launcher/psb_key_settings_test.cpp)
    target_link_libraries(psb_key_settings_test PRIVATE entis_psb_keys)
    if(ENTISGLS_BUILD_DIAGNOSTICS)
    add_executable(motion_psb_key_callback_test EXCLUDE_FROM_ALL tests/probes/emote/tjs_runtime/psb_key_resolver_probe.cpp)
    target_link_libraries(motion_psb_key_callback_test PRIVATE motion_apk_runtime)
    endif()
    add_executable(make_csx_fixture EXCLUDE_FROM_ALL tests/fixtures/make_csx_fixture.cpp "${LEGACY_HEAP_SDK}"
        native/compatibility/games/compatibility_profiles.cpp native/platform/sdl/game_font_aliases.cpp native/platform/sdl/opentype_font.cpp)
    target_link_libraries(make_csx_fixture PRIVATE legacy_objects legacy_foundation gls4_sdl motion_apk_runtime freetype)
    add_executable(launcher_config_test EXCLUDE_FROM_ALL tests/integration/launcher/game_config_test.cpp native/launcher/game_config.cpp)
    target_link_libraries(launcher_config_test PRIVATE gls4_sdl)
    add_executable(launcher_fonts_test EXCLUDE_FROM_ALL tests/integration/launcher/generic_fonts_test.cpp
        native/platform/sdl/game_font_aliases.cpp native/platform/sdl/opentype_font.cpp native/compatibility/games/compatibility_profiles.cpp)
    target_link_libraries(launcher_fonts_test PRIVATE gls4_sdl freetype)
endif()
