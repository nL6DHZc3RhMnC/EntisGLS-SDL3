# SDL owns platform services. The supplied SDK remains a read-only input.
include("${CMAKE_CURRENT_LIST_DIR}/sdl3_dependency.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sdl3_fonts.cmake")
set(STUDYSTEADY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${STUDYSTEADY_ROOT}/tools/prepare_sdl_sdk.py"
    "${STUDYSTEADY_ROOT}/tools/prepare_sdl_window.py"
    "${STUDYSTEADY_ROOT}/tools/prepare_sdl_system.py"
    "${STUDYSTEADY_ROOT}/tools/prepare_sdl_sync.py")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${STUDYSTEADY_ROOT}/tools/prepare_sdl_graphics.py")
set(SDL_SDK_OVERLAY "${CMAKE_CURRENT_BINARY_DIR}/sdk-sdl")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${STUDYSTEADY_ROOT}/tools/prepare_sdl_sdk.py"
    --output "${SDL_SDK_OVERLAY}" COMMAND_ERROR_IS_FATAL ANY)
set(SDL_SDK_INCLUDES
    "${SDL_SDK_OVERLAY}/include" "${SDL_SDK_OVERLAY}/Include/common"
    "${SDL_SDK_OVERLAY}/Include/unix" "${SDL_SDK_OVERLAY}/Include/opengl"
    "${STUDYSTEADY_ROOT}/native"
    "${ENTIS_ROOT}/Include/common" "${ENTIS_ROOT}/Include/unix"
    "${ENTIS_ROOT}/Include/opengl")
add_library(study_sdl_platform INTERFACE)
target_include_directories(study_sdl_platform INTERFACE ${SDL_SDK_INCLUDES})
target_compile_definitions(study_sdl_platform INTERFACE
    STUDYSTEADY_PLATFORM_SDL3=1 PLATFORM_ANDROID=0 ANDROID_NDK_VER=27)
target_compile_options(study_sdl_platform INTERFACE
    -Wno-invalid-offsetof -Wno-deprecated-declarations -fno-delete-null-pointer-checks)
# Mobile GLES and desktop OpenGL are graphics capabilities, not SDK OS switches.
if(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(study_sdl_platform INTERFACE
        STUDYSTEADY_GL_ES=1 STUDYSTEADY_GL_API_LEVEL=18 __API_OPEN_GL_ES__=2)
endif()
target_link_libraries(study_sdl_platform INTERFACE SDL3::SDL3)

set(ENTIS_SOURCE_GROUPS common/esl common/sakura
    common/sakuracl/erisa common/sakuragl opengl/sakuragl
    common/sakuragl/erisa common/sakuragl/media common/sakuragl/sgl2d
    common/sakuragl/sgl3d common/sakuragl/window common/sakuraglx
    common/sakuraglx/ui common/sakuraglx/sprite common/sakuraglx/render
    common/sakuraglx/extra common/glscs common/rosetta common/loquaty common/antirrhinum)
set(SDL_SDK_SOURCES)
foreach(group IN LISTS ENTIS_SOURCE_GROUPS)
    file(GLOB sources CONFIGURE_DEPENDS "${ENTIS_ROOT}/Source/${group}/*.cpp")
    foreach(source IN LISTS sources)
        file(RELATIVE_PATH relative "${ENTIS_ROOT}" "${source}")
        if(relative MATCHES "ssys_(synchronism|std_ui|file)\\.cpp$")
            continue()
        endif()
        if(EXISTS "${SDL_SDK_OVERLAY}/${relative}")
            list(APPEND SDL_SDK_SOURCES "${SDL_SDK_OVERLAY}/${relative}")
        else()
            list(APPEND SDL_SDK_SOURCES "${source}")
        endif()
    endforeach()
endforeach()
add_library(gls4_sdl STATIC ${SDL_SDK_SOURCES}
    "${ENTIS_ROOT}/Source/common/sakuragl/sgl3d/neon/sgl3d_matrix_neon.cpp"
    "${ENTIS_ROOT}/Source/common/sakuraglx/render/neon/sglx3d_collision_neon.cpp"
    "${SDL_SDK_OVERLAY}/source/ssys_synchronism.cpp"
    "${SDL_SDK_OVERLAY}/source/sgl_generic_window.cpp"
    "${SDL_SDK_OVERLAY}/system/ssys_stdapi.cpp"
    "${SDL_SDK_OVERLAY}/system/ssys_std_ui.cpp"
    "${SDL_SDK_OVERLAY}/system/ssys_file.cpp"
    native/platform/sdl/synchronization.cpp native/platform/sdl/memory_info.cpp
    native/platform/sdl/system.cpp native/platform/sdl/game_file_opener.cpp native/platform/sdl/sdl_pcm_stream.cpp
    native/platform/sdl/device_volume.cpp
    native/platform/sdl/sdl_sound_player.cpp native/platform/sdl/image_codec.cpp
    native/platform/sdl/sdk_image_codec.cpp)
target_include_directories(gls4_sdl PRIVATE "${STUDYSTEADY_ROOT}/vendor/official-tinygltf")
target_link_libraries(gls4_sdl PUBLIC study_sdl_platform entis_game_files)
if(ANDROID)
    target_link_libraries(gls4_sdl PUBLIC GLESv1_CM GLESv2 GLESv3 log)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    find_library(STUDY_OPENGLES_FRAMEWORK OpenGLES REQUIRED)
    target_link_libraries(gls4_sdl PUBLIC "${STUDY_OPENGLES_FRAMEWORK}")
elseif(APPLE)
    find_library(STUDY_OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(gls4_sdl PUBLIC "${STUDY_OPENGL_FRAMEWORK}")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(gls4_sdl PUBLIC OpenGL::GL OpenGL::GLU)
endif()
file(GLOB LOQUATY_SOURCES CONFIGURE_DEPENDS "${STUDYSTEADY_ROOT}/vendor/official-loquaty/Loquaty/source/*.cpp")
list(REMOVE_ITEM LOQUATY_SOURCES "${STUDYSTEADY_ROOT}/vendor/official-loquaty/Loquaty/source/loquaty_file.cpp")
list(APPEND LOQUATY_SOURCES "${SDL_SDK_OVERLAY}/loquaty/loquaty_file.cpp")
add_library(loquaty STATIC ${LOQUATY_SOURCES})
target_include_directories(loquaty PUBLIC "${STUDYSTEADY_ROOT}/vendor/official-loquaty/Loquaty/include")
target_link_libraries(gls4_sdl PUBLIC loquaty)
set_target_properties(gls4_sdl loquaty PROPERTIES POSITION_INDEPENDENT_CODE ON)
include("${CMAKE_CURRENT_LIST_DIR}/legacy_runtime.cmake")
add_subdirectory(native/motion_bridge/tjs_runtime motion-tjs EXCLUDE_FROM_ALL)
target_link_libraries(legacy_objects PUBLIC motion_apk_runtime)
set(LEGACY_HEAP_SDK "${CMAKE_CURRENT_BINARY_DIR}/legacy_heap_sdk/glscs_sakura2_obj_heap.cpp")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${STUDYSTEADY_ROOT}/tools/motion_prepare_heap_sdk.py"
    --output "${LEGACY_HEAP_SDK}" COMMAND_ERROR_IS_FATAL ANY)
set(SDL_APP_SOURCES native/launcher/psb_key_dialog.cpp native/launcher/game_config.cpp native/launcher/known_game.cpp
    native/launcher/save_directory.cpp
    native/launcher/compatibility_profiles.cpp native/sdl_main.cpp "${LEGACY_HEAP_SDK}"
    native/platform/sdl/game_font_aliases.cpp
    native/platform/sdl/opentype_font.cpp
    native/legacy_file_probe.cpp native/legacy_core_probe.cpp native/legacy_media_probe.cpp
    native/legacy_runner.cpp native/legacy_input_probe.cpp native/legacy_window_probe.cpp
    native/legacy_movie_window_probe.cpp native/legacy_setup_probe.cpp)
if(ANDROID)
    add_library(studysteady_sdl SHARED ${SDL_APP_SOURCES} native/platform/sdl/android_game_files.cpp)
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
                MACOSX_BUNDLE_INFO_PLIST "${STUDYSTEADY_ROOT}/native/platform/sdl/ios/Info.plist"
                MACOSX_BUNDLE_GUI_IDENTIFIER "${ENTISGLS_IOS_BUNDLE_IDENTIFIER}"
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${ENTISGLS_IOS_BUNDLE_IDENTIFIER}"
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
                XCODE_ATTRIBUTE_SUPPORTS_MACCATALYST NO
                XCODE_ATTRIBUTE_ENABLE_BITCODE NO)
            target_sources(studysteady_sdl PRIVATE native/platform/sdl/ios_launcher.mm
                native/platform/sdl/ios/LaunchScreen.storyboard)
            set_source_files_properties(native/platform/sdl/ios_launcher.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
            set_source_files_properties(native/platform/sdl/ios/LaunchScreen.storyboard PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources)
            find_library(STUDY_UIKIT_FRAMEWORK UIKit REQUIRED)
            find_library(STUDY_FOUNDATION_FRAMEWORK Foundation REQUIRED)
            target_link_libraries(studysteady_sdl PRIVATE
                "${STUDY_UIKIT_FRAMEWORK}" "${STUDY_FOUNDATION_FRAMEWORK}")
        else()
            set_target_properties(studysteady_sdl PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST "${STUDYSTEADY_ROOT}/native/platform/sdl/macos/Info.plist")
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
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(sdl_system_test EXCLUDE_FROM_ALL native/platform/sdl/tests/system_test.cpp)
    target_link_libraries(sdl_system_test PRIVATE gls4_sdl)
endif()

if(NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(game_save_directory_test EXCLUDE_FROM_ALL native/launcher/tests/save_directory_test.cpp
        native/launcher/save_directory.cpp native/legacy_atomic_path.cpp)
    target_compile_definitions(game_save_directory_test PRIVATE STUDYSTEADY_PLATFORM_SDL3=1)
    target_link_libraries(game_save_directory_test PRIVATE entis_game_files)
    add_executable(psb_key_resolver_test EXCLUDE_FROM_ALL native/launcher/tests/psb_key_resolver_test.cpp)
    target_link_libraries(psb_key_resolver_test PRIVATE entis_psb_keys)
    add_executable(psb_key_settings_test EXCLUDE_FROM_ALL native/launcher/tests/psb_key_settings_test.cpp)
    target_link_libraries(psb_key_settings_test PRIVATE entis_psb_keys)
    add_executable(motion_psb_key_callback_test EXCLUDE_FROM_ALL native/motion_bridge/tjs_runtime/psb_key_resolver_probe.cpp)
    target_link_libraries(motion_psb_key_callback_test PRIVATE motion_apk_runtime)
    add_executable(make_csx_fixture EXCLUDE_FROM_ALL native/launcher/tests/make_csx_fixture.cpp "${LEGACY_HEAP_SDK}"
        native/launcher/compatibility_profiles.cpp native/platform/sdl/game_font_aliases.cpp native/platform/sdl/opentype_font.cpp)
    target_link_libraries(make_csx_fixture PRIVATE legacy_objects legacy_foundation gls4_sdl motion_apk_runtime freetype)
    add_executable(launcher_config_test EXCLUDE_FROM_ALL native/launcher/tests/game_config_test.cpp native/launcher/game_config.cpp)
    target_link_libraries(launcher_config_test PRIVATE gls4_sdl)
    add_executable(launcher_fonts_test EXCLUDE_FROM_ALL native/launcher/generic_fonts_test.cpp
        native/platform/sdl/game_font_aliases.cpp native/platform/sdl/opentype_font.cpp native/launcher/compatibility_profiles.cpp)
    target_link_libraries(launcher_fonts_test PRIVATE gls4_sdl freetype)
endif()
