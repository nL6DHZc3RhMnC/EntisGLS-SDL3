# Platform entry points, resource packaging and application composition.
set(SDL_APP_SOURCES native/launcher/psb_key_dialog.cpp native/launcher/game_config.cpp native/compatibility/games/known_game.cpp
    native/io/save_directory.cpp
    native/compatibility/games/compatibility_profiles.cpp apps/launcher/main.cpp "${LEGACY_HEAP_SDK}"
    native/platform/sdl/game_font_aliases.cpp
    native/platform/sdl/opentype_font.cpp
    native/launcher/runtime_session.cpp)
if(ANDROID)
    add_library(studysteady_sdl SHARED ${SDL_APP_SOURCES} native/platform/android/android_game_files.cpp)
    set_target_properties(studysteady_sdl PROPERTIES OUTPUT_NAME main)
else()
    add_executable(studysteady_sdl ${SDL_APP_SOURCES})
    set_target_properties(studysteady_sdl PROPERTIES OUTPUT_NAME entisgls-launcher)
    if(APPLE)
        set_target_properties(studysteady_sdl PROPERTIES MACOSX_BUNDLE TRUE
            OUTPUT_NAME EntisGLSLauncher)
        if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            enable_language(OBJCXX)
            set(ENTISGLS_IOS_BUNDLE_IDENTIFIER "io.entisgls.launcher" CACHE STRING "iOS bundle identifier")
            if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
                set(ENTISGLS_IOS_PLATFORM iPhoneSimulator)
            else()
                set(ENTISGLS_IOS_PLATFORM iPhoneOS)
            endif()
            set_target_properties(studysteady_sdl PROPERTIES
                OBJCXX_STANDARD 17
                OBJCXX_STANDARD_REQUIRED YES
                MACOSX_BUNDLE_INFO_PLIST "${STUDYSTEADY_ROOT}/apps/ios/Info.plist"
                MACOSX_BUNDLE_GUI_IDENTIFIER "${ENTISGLS_IOS_BUNDLE_IDENTIFIER}"
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${ENTISGLS_IOS_BUNDLE_IDENTIFIER}"
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
                XCODE_ATTRIBUTE_SUPPORTS_MACCATALYST NO
                XCODE_ATTRIBUTE_ENABLE_BITCODE NO)
            target_sources(studysteady_sdl PRIVATE apps/ios/ios_launcher.mm
                native/platform/ios/ios_gl_context.mm
                native/platform/ios/window_orientation.mm
                tests/integration/ios/ios_presentation_smoke.cpp
                apps/ios/LaunchScreen.storyboard)
            set_source_files_properties(apps/ios/ios_launcher.mm
                native/platform/ios/ios_gl_context.mm native/platform/ios/window_orientation.mm
                PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
            set_source_files_properties(apps/ios/LaunchScreen.storyboard PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources)
            find_library(STUDY_UIKIT_FRAMEWORK UIKit REQUIRED)
            find_library(STUDY_FOUNDATION_FRAMEWORK Foundation REQUIRED)
            target_link_libraries(studysteady_sdl PRIVATE
                "${STUDY_UIKIT_FRAMEWORK}" "${STUDY_FOUNDATION_FRAMEWORK}")
        else()
            set_target_properties(studysteady_sdl PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST "${STUDYSTEADY_ROOT}/apps/macos/Info.plist")
        endif()
        file(GLOB_RECURSE SDL_FONT_ASSETS CONFIGURE_DEPENDS
            "${STUDYSTEADY_ROOT}/assets/fonts/*" "${STUDYSTEADY_ROOT}/assets/licenses/*"
            "${STUDYSTEADY_ROOT}/assets/compatibility/*")
        foreach(asset IN LISTS SDL_FONT_ASSETS)
            file(RELATIVE_PATH relative "${STUDYSTEADY_ROOT}/assets" "${asset}")
            get_filename_component(directory "${relative}" DIRECTORY)
            target_sources(studysteady_sdl PRIVATE "${asset}")
            set_source_files_properties("${asset}" PROPERTIES
                HEADER_FILE_ONLY TRUE MACOSX_PACKAGE_LOCATION "Resources/assets/${directory}")
        endforeach()
    else()
        add_custom_command(TARGET studysteady_sdl POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${STUDYSTEADY_ROOT}/assets" "$<TARGET_FILE_DIR:studysteady_sdl>/assets")
    endif()
endif()
target_link_libraries(studysteady_sdl PRIVATE legacy_objects legacy_foundation gls4_sdl motion_apk_runtime freetype z)
entis_attach_cotopha_diagnostics(studysteady_sdl)
