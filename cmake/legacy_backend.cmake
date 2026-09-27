# Optional pre-SDL Android backend and standalone SDK inspection tools.
execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/sdk/prepare_psb_port.py
    COMMAND_ERROR_IS_FATAL ANY)
set(MOTION_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/kirikiroid2/motionplayer)
add_library(motion_core STATIC
    ${MOTION_SOURCE}/EmoteAngleController.cpp
    ${MOTION_SOURCE}/EmoteVarController.cpp
    ${MOTION_SOURCE}/EmoteSelectorController.cpp
    ${MOTION_SOURCE}/EmoteLoopController.cpp
    ${MOTION_SOURCE}/EmoteBlinkRng.cpp)
set_target_properties(motion_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(motion_core PUBLIC ${MOTION_SOURCE})
add_library(psb_reader STATIC build/generated/psb/PSBRawFile.cpp)
set_target_properties(psb_reader PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(psb_reader PUBLIC native build/generated/psb)
target_link_libraries(psb_reader PUBLIC z)
if(ENTISGLS_BUILD_DIAGNOSTICS)
    add_executable(psb_probe tests/probes/emote/psb_probe.cpp)
    target_link_libraries(psb_probe PRIVATE psb_reader motion_core)
endif()
if(ANDROID)
    option(LEGACY_COMPILE_OBJECTS "Compile the traditional object runtime" ON)
    if(NOT LEGACY_COMPILE_OBJECTS)
        message(FATAL_ERROR "The APK now requires LEGACY_COMPILE_OBJECTS=ON")
    endif()
    include(cmake/legacy_runtime.cmake)
    add_subdirectory(native/extensions/emote/tjs_runtime motion-tjs EXCLUDE_FROM_ALL)
    # Only JNI is an exported APK interface. Hidden static Player symbols let
    # the linker discard unused renderer entry points until their real backend
    # is linked; an actual call to a missing backend still fails the link.
    set_target_properties(motion_player_cpu motion_psb_tjs motion_ncb motion_tjs motion_apk_runtime
        PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
    include(cmake/official_entis.cmake)
    add_library(cpufeatures STATIC "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
    set_target_properties(cpufeatures PROPERTIES POSITION_INDEPENDENT_CODE ON)
    file(GLOB JNI_SOURCES "${ENTIS_ROOT}/Source/android/gls4jclass/*.cpp")
    # Keep the traditional paint-freeze gate before the SDK takes its UI lock.
    # OnDraw is nonvirtual, so adapt only its JNI call in a generated copy.
    set(ANDROID_WINDOW_JNI "${CMAKE_CURRENT_BINARY_DIR}/official_jni/VirtualWindow_java.cpp")
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/sdk/prepare_android_jni.py"
        --output "${ANDROID_WINDOW_JNI}" COMMAND_ERROR_IS_FATAL ANY)
    list(REMOVE_ITEM JNI_SOURCES "${ENTIS_ROOT}/Source/android/gls4jclass/VirtualWindow_java.cpp")
    list(APPEND JNI_SOURCES "${ANDROID_WINDOW_JNI}")
    # Supply the complete checked ObjectHeap object before the vendor archive.
    # The matching archive member is not extracted; original SDK files stay intact.
    set(LEGACY_HEAP_SDK "${CMAKE_CURRENT_BINARY_DIR}/legacy_heap_sdk/glscs_sakura2_obj_heap.cpp")
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/sdk/motion_prepare_heap_sdk.py"
        --output "${LEGACY_HEAP_SDK}" COMMAND_ERROR_IS_FATAL ANY)
    add_library(entisgls4 SHARED ${JNI_SOURCES} apps/android/native/legacy_main.cpp native/platform/android/psb_jni.cpp
        "${LEGACY_HEAP_SDK}"
        native/launcher/runtime_session.cpp native/platform/android/legacy_android_audio.cpp)
    target_include_directories(entisgls4 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/native"
        ${ENTIS_ROOT}/Include/common ${ENTIS_ROOT}/Include/unix
        ${ENTIS_ROOT}/Include/opengl ${ENTIS_ROOT}/Include/android
        ${ENTIS_ROOT}/Include/android/gls4jclass)
    target_compile_definitions(entisgls4 PRIVATE PLATFORM_ANDROID
        ANDROID_NDK_VER=27 ANDROID_API_LEVEL=29 __arm__=1)
    target_compile_options(entisgls4 PRIVATE -Wno-invalid-offsetof -Wno-deprecated-declarations
        -fno-delete-null-pointer-checks)
    target_link_options(entisgls4 PRIVATE -Wl,--gc-sections)
    target_link_libraries(entisgls4 PRIVATE legacy_objects legacy_foundation gls4 loquaty cpufeatures motion_apk_runtime motion_core log EGL GLESv1_CM GLESv2 GLESv3 android z)
    entis_attach_cotopha_diagnostics(entisgls4)
    return()
endif()
if(ENTISGLS_BUILD_DIAGNOSTICS)
add_executable(erisan_decode
    tests/probes/sdk/erisan_decode.cpp
    ${ENTIS_ROOT}/Source/common/sakuragl/erisa/sgl_erisa_decode_context.cpp
    ${ENTIS_ROOT}/Source/common/sakuragl/erisa/sgl_erisa_context_model.cpp)
target_include_directories(erisan_decode PRIVATE
    native/compatibility/sdk/erisa ${ENTIS_ROOT}/Include/common)
endif()
