cmake_minimum_required(VERSION 3.20)
project(StudySteadyMotionTJS LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
get_filename_component(STUDYSTEADY_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
option(STUDYSTEADY_SDL3 "Build the SDL3 platform backend" OFF)
add_library(motion_platform INTERFACE)
target_include_directories(motion_platform INTERFACE ${STUDYSTEADY_ROOT}/native)
if(STUDYSTEADY_SDL3)
    if(NOT TARGET SDL3::SDL3)
        include(${STUDYSTEADY_ROOT}/cmake/sdl3_dependency.cmake)
    endif()
    target_compile_definitions(motion_platform INTERFACE STUDYSTEADY_PLATFORM_SDL3=1)
    target_link_libraries(motion_platform INTERFACE SDL3::SDL3)
endif()
set(TJS_SOURCE "${STUDYSTEADY_ROOT}/vendor/kirikiroid2/tjs2")
set(DEPS "${STUDYSTEADY_ROOT}/vendor/motion-deps")
set(TJS_BOOTSTRAP_BOOST "" CACHE PATH "Temporary read-only Boost headers to collect actual include closure")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TEST OFF CACHE BOOL "" FORCE)
set(INSTALL_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${DEPS}/oniguruma ${CMAKE_CURRENT_BINARY_DIR}/oniguruma)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NOT TJS_BISON AND DEFINED ENV{TJS_BISON})
    set(TJS_BISON "$ENV{TJS_BISON}" CACHE FILEPATH "Host Bison executable (3.8.2 or newer)" FORCE)
endif()
find_program(TJS_BISON NAMES bison REQUIRED NO_CMAKE_FIND_ROOT_PATH)
execute_process(COMMAND "${TJS_BISON}" --version
    OUTPUT_VARIABLE TJS_BISON_VERSION_TEXT COMMAND_ERROR_IS_FATAL ANY)
string(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?" TJS_BISON_VERSION "${TJS_BISON_VERSION_TEXT}")
if(NOT TJS_BISON_VERSION OR TJS_BISON_VERSION VERSION_LESS "3.8.2")
    message(FATAL_ERROR "TJS requires Bison 3.8.2 or newer; found ${TJS_BISON_VERSION} at ${TJS_BISON}. Set TJS_BISON to a supported host executable.")
endif()
set(GENERATED ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED})
set(TJS_PARSERS)
foreach(NAME tjs tjsdate tjspp)
    if(NOT EXISTS ${GENERATED}/${NAME}.tab.cpp OR ${TJS_SOURCE}/bison/${NAME}.y IS_NEWER_THAN ${GENERATED}/${NAME}.tab.cpp)
        execute_process(COMMAND ${TJS_BISON} ${TJS_SOURCE}/bison/${NAME}.y WORKING_DIRECTORY ${GENERATED} COMMAND_ERROR_IS_FATAL ANY)
    endif()
    list(APPEND TJS_PARSERS ${GENERATED}/${NAME}.tab.cpp)
endforeach()

add_library(motion_apk_runtime STATIC EXCLUDE_FROM_ALL motion_apk_runtime.cpp state_codec.cpp)
target_link_libraries(motion_apk_runtime PUBLIC motion_player_scene)
target_include_directories(motion_apk_runtime PUBLIC ${CMAKE_CURRENT_LIST_DIR})
if(NOT STUDYSTEADY_SDL3 AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # This probe specifically tests a worker-owned runtime. SDL requires the
    # runtime and its shared GL context to live on the main thread instead.
    add_executable(motion_runtime_probe EXCLUDE_FROM_ALL runtime_probe.cpp)
    target_link_libraries(motion_runtime_probe PRIVATE motion_apk_runtime)
    if(ANDROID)
        target_link_options(motion_runtime_probe PRIVATE -Wl,--gc-sections)
    elseif(APPLE)
        target_link_options(motion_runtime_probe PRIVATE -Wl,-dead_strip)
    endif()
endif()
if(NOT EXISTS ${GENERATED}/tjsDateWordMap.inc OR ${TJS_SOURCE}/script/dp_word_table.txt IS_NEWER_THAN ${GENERATED}/tjsDateWordMap.inc)
    execute_process(COMMAND ${Python3_EXECUTABLE} create_world_map.py ${GENERATED}/tjsDateWordMap.inc WORKING_DIRECTORY ${TJS_SOURCE}/script COMMAND_ERROR_IS_FATAL ANY)
endif()
file(GLOB TJS_FILES CONFIGURE_DEPENDS ${TJS_SOURCE}/*.cpp)
list(REMOVE_ITEM TJS_FILES ${TJS_SOURCE}/tjsConfig.cpp)
add_library(motion_tjs STATIC ${TJS_FILES} ${TJS_PARSERS})
target_include_directories(motion_tjs PUBLIC ${TJS_SOURCE} ${GENERATED} ${DEPS}/include PRIVATE ${STUDYSTEADY_ROOT}/vendor/kirikiroid2/tjs_support)
if(TJS_BOOTSTRAP_BOOST)
    target_include_directories(motion_tjs SYSTEM PUBLIC ${TJS_BOOTSTRAP_BOOST})
endif()
target_compile_definitions(motion_tjs PUBLIC FMT_HEADER_ONLY SPDLOG_FMT_EXTERNAL)
target_link_libraries(motion_tjs PUBLIC onig motion_platform)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_tjs_probe tjs_probe.cpp)
    target_link_libraries(motion_tjs_probe PRIVATE motion_tjs)
endif()
execute_process(COMMAND ${Python3_EXECUTABLE} ${STUDYSTEADY_ROOT}/tools/motion_prepare_tjs.py COMMAND_ERROR_IS_FATAL ANY)
set(NCB_GENERATED ${STUDYSTEADY_ROOT}/build/generated/motion-ncbind)
add_library(motion_ncb STATIC ${NCB_GENERATED}/ncbind.cpp tjs_host.cpp)
target_include_directories(motion_ncb PUBLIC ${NCB_GENERATED} ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(motion_ncb PUBLIC motion_tjs)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_ncb_probe ncb_probe.cpp)
    target_link_libraries(motion_ncb_probe PRIVATE motion_ncb)
endif()
target_sources(motion_tjs PRIVATE ${NCB_GENERATED}/locale.cpp ${NCB_GENERATED}/tjsConfig.cpp)
execute_process(COMMAND ${Python3_EXECUTABLE} ${STUDYSTEADY_ROOT}/tools/motion_prepare_tjs_psb.py COMMAND_ERROR_IS_FATAL ANY)
set(PSB_TJS_GENERATED ${STUDYSTEADY_ROOT}/build/generated/motion-tjs-psb)
add_library(motion_psb_tjs STATIC ${PSB_TJS_GENERATED}/psbfile/PSBRawFile.cpp ${PSB_TJS_GENERATED}/psbfile/PSBFile.cpp)
target_include_directories(motion_psb_tjs PUBLIC ${PSB_TJS_GENERATED} PRIVATE ${STUDYSTEADY_ROOT}/native)
target_link_libraries(motion_psb_tjs PUBLIC motion_ncb z)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_psb_tjs_probe psb_tjs_probe.cpp)
    target_include_directories(motion_psb_tjs_probe PRIVATE ${STUDYSTEADY_ROOT}/native)
    target_link_libraries(motion_psb_tjs_probe PRIVATE motion_psb_tjs)
endif()
execute_process(COMMAND ${Python3_EXECUTABLE} ${STUDYSTEADY_ROOT}/tools/motion_prepare_player.py COMMAND_ERROR_IS_FATAL ANY)
set(PLAYER_GENERATED ${STUDYSTEADY_ROOT}/build/generated/motion-player)
add_library(motion_player_cpu STATIC EXCLUDE_FROM_ALL
    ${PLAYER_GENERATED}/PlayerCore.cpp
    ${PLAYER_GENERATED}/PlayerVariable.cpp
    ${PLAYER_GENERATED}/PlayerMotionLoad.cpp
    ${PLAYER_GENERATED}/PlayerUpdateLayerEval.cpp
    ${PLAYER_GENERATED}/PlayerFrameProgress.cpp
    ${PLAYER_GENERATED}/NodeTree.cpp
    ${PLAYER_GENERATED}/MotionNodeBridge.cpp
    ${PLAYER_GENERATED}/RuntimeSupport.cpp)
target_include_directories(motion_player_cpu PUBLIC ${PLAYER_GENERATED} ${STUDYSTEADY_ROOT}/vendor/kirikiroid2/tjs_support)
target_compile_options(motion_player_cpu PRIVATE -ffunction-sections -fdata-sections)
target_link_libraries(motion_player_cpu PUBLIC motion_psb_tjs)
target_sources(motion_player_cpu PRIVATE ${PLAYER_GENERATED}/SeparateLayerOwnership.cpp ${PLAYER_GENERATED}/ResourceManagerCpu.cpp psb_storage.cpp)
target_include_directories(motion_player_cpu PRIVATE ${STUDYSTEADY_ROOT}/native)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_player_probe EXCLUDE_FROM_ALL player_probe.cpp)
    target_link_libraries(motion_player_probe PRIVATE motion_player_cpu)
    if(ANDROID)
        target_link_options(motion_player_probe PRIVATE -Wl,--gc-sections)
    elseif(APPLE)
        target_link_options(motion_player_probe PRIVATE -Wl,-dead_strip)
    endif()
endif()
target_sources(motion_player_cpu PRIVATE ${PLAYER_GENERATED}/PlayerQueryCpu.cpp ${STUDYSTEADY_ROOT}/vendor/kirikiroid2/tjs_support/ComplexRect.cpp)
target_sources(motion_player_cpu PRIVATE bezier_runtime.cpp mesh_combinator.cpp player_mesh_bridge.cpp ${PLAYER_GENERATED}/PlayerTimelineColor.cpp)
add_library(motion_player_scene STATIC EXCLUDE_FROM_ALL
    texture_bridge.cpp
    ${PLAYER_GENERATED}/PlayerTimelineCore.cpp
    ${PLAYER_GENERATED}/PlayerResourceWin.cpp
    ${PLAYER_GENERATED}/ResourceManagerBlank.cpp
    ${PLAYER_GENERATED}/PlayerUpdateLayers.cpp
    ${PLAYER_GENERATED}/PlayerUpdateGeometry.cpp
    ${PLAYER_GENERATED}/PlayerUpdateChildMotion.cpp
    ${PLAYER_GENERATED}/PlayerUpdateParticles.cpp
    ${PLAYER_GENERATED}/PlayerUpdateAnchor.cpp
    ${PLAYER_GENERATED}/PlayerRenderItemsScene.cpp)
target_sources(motion_player_scene PRIVATE ${PLAYER_GENERATED}/PlayerDrawTransform.cpp)
target_sources(motion_player_scene PRIVATE ${PLAYER_GENERATED}/PlayerLayerQueryScene.cpp ${PLAYER_GENERATED}/EmotePlayerAccessors.cpp)
target_sources(motion_player_scene PRIVATE
    ${PLAYER_GENERATED}/EmoteEngine.cpp
    ${PLAYER_GENERATED}/EmoteVarController.cpp
    ${PLAYER_GENERATED}/EmoteAngleController.cpp
    ${PLAYER_GENERATED}/EmoteBlinkController.cpp
    ${PLAYER_GENERATED}/EmoteBlinkRng.cpp
    ${PLAYER_GENERATED}/EmoteEyebrowController.cpp
    ${PLAYER_GENERATED}/EmoteMouthController.cpp
    ${PLAYER_GENERATED}/EmoteSelectorController.cpp
    ${PLAYER_GENERATED}/EmoteLoopController.cpp
    ${PLAYER_GENERATED}/EmoteMeshResolver.cpp
    ${PLAYER_GENERATED}/EmoteSpring.cpp
    ${PLAYER_GENERATED}/EmoteWindEmitter.cpp)
target_compile_options(motion_player_scene PRIVATE -ffunction-sections -fdata-sections)
target_link_libraries(motion_player_scene PUBLIC motion_player_cpu)
if(ANDROID OR STUDYSTEADY_SDL3)
    add_library(motion_gles_scene STATIC EXCLUDE_FROM_ALL
        gles_render_manager.cpp
        ${PLAYER_GENERATED}/MotionRenderBackendNative.cpp
        ${PLAYER_GENERATED}/GlesSceneItems.cpp)
    target_link_libraries(motion_gles_scene PUBLIC motion_player_scene)
    target_link_libraries(motion_apk_runtime PUBLIC motion_gles_scene)
    if(NOT STUDYSTEADY_SDL3)
        target_link_libraries(motion_player_scene PUBLIC GLESv2)
        target_link_libraries(motion_gles_scene PUBLIC GLESv2)
        target_link_libraries(motion_apk_runtime PUBLIC EGL)
    endif()
endif()
if(ANDROID AND NOT STUDYSTEADY_SDL3)
    add_executable(motion_runtime_gles_probe EXCLUDE_FROM_ALL runtime_gles_probe.cpp)
    target_link_libraries(motion_runtime_gles_probe PRIVATE motion_apk_runtime EGL GLESv2)
    target_link_options(motion_runtime_gles_probe PRIVATE -Wl,--gc-sections)
    add_executable(motion_scene_gles_probe EXCLUDE_FROM_ALL scene_probe.cpp)
    target_compile_definitions(motion_scene_gles_probe PRIVATE MOTION_SCENE_GLES)
    target_link_libraries(motion_scene_gles_probe PRIVATE motion_gles_scene EGL)
    target_link_options(motion_scene_gles_probe PRIVATE -Wl,--gc-sections)
endif()
if(STUDYSTEADY_SDL3 AND NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # A desktop GL 2.1 context and an original PSB argument are required.
    # Mobile SDL packaging uses its platform bootstrap and GLES window instead.
    add_executable(motion_runtime_sdl_probe EXCLUDE_FROM_ALL ${STUDYSTEADY_ROOT}/tools/test_sdl_motion.cpp)
    target_link_libraries(motion_runtime_sdl_probe PRIVATE motion_apk_runtime SDL3::SDL3)
    add_executable(motion_gl_window_lease_probe EXCLUDE_FROM_ALL ${STUDYSTEADY_ROOT}/tools/test_sdl_gl_window_lease.cpp)
    target_link_libraries(motion_gl_window_lease_probe PRIVATE motion_platform)
    if(APPLE)
        target_link_options(motion_runtime_sdl_probe PRIVATE -Wl,-dead_strip)
    endif()
endif()
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_scene_probe EXCLUDE_FROM_ALL scene_probe.cpp)
    target_link_libraries(motion_scene_probe PRIVATE motion_player_scene)
    if(ANDROID)
        target_link_options(motion_scene_probe PRIVATE -Wl,--gc-sections)
    elseif(APPLE)
        target_link_options(motion_scene_probe PRIVATE -Wl,-dead_strip)
    endif()
endif()
# The Windows reference uses separate SSE multiply/add and add/sub operations.
# Prevent ARM64 contraction from silently changing the new-format mesh kernel.
set_source_files_properties(mesh_combinator.cpp PROPERTIES COMPILE_OPTIONS "-ffp-contract=off")
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_executable(motion_mesh_probe EXCLUDE_FROM_ALL mesh_probe.cpp)
    target_link_libraries(motion_mesh_probe PRIVATE motion_player_cpu)
    add_executable(motion_timeline_probe EXCLUDE_FROM_ALL timeline_probe.cpp)
    target_link_libraries(motion_timeline_probe PRIVATE motion_player_cpu)
    foreach(PROBE motion_mesh_probe motion_timeline_probe)
        if(ANDROID)
            target_link_options(${PROBE} PRIVATE -Wl,--gc-sections)
        elseif(APPLE)
            target_link_options(${PROBE} PRIVATE -Wl,-dead_strip)
        endif()
    endforeach()
endif()
