include("${CMAKE_CURRENT_LIST_DIR}/sdk_generation.cmake")
entis_track_sdk_generation()
# SDL owns platform services. The supplied SDK remains a read-only input.
include("${CMAKE_CURRENT_LIST_DIR}/sdl3_dependency.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sdl3_fonts.cmake")
set(STUDYSTEADY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_sdk.py"
    "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_window.py"
    "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_system.py"
    "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_sync.py")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_graphics.py")
set(SDL_SDK_OVERLAY "${CMAKE_CURRENT_BINARY_DIR}/sdk-sdl")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${STUDYSTEADY_ROOT}/tools/sdk/prepare_sdl_sdk.py"
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
    native/platform/sdl/mobile_orientation.cpp
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
add_subdirectory(native/extensions/emote/tjs_runtime motion-tjs EXCLUDE_FROM_ALL)
target_link_libraries(legacy_objects PUBLIC motion_apk_runtime)
set(LEGACY_HEAP_SDK "${CMAKE_CURRENT_BINARY_DIR}/legacy_heap_sdk/glscs_sakura2_obj_heap.cpp")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${STUDYSTEADY_ROOT}/tools/sdk/motion_prepare_heap_sdk.py"
    --output "${LEGACY_HEAP_SDK}" COMMAND_ERROR_IS_FATAL ANY)
include("${CMAKE_CURRENT_LIST_DIR}/launcher_app.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/runtime_tests.cmake")
