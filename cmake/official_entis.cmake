# Build the official SDK sources; no binary libraries from the retired SDK tree.
# Source groups mirror EntisGLS4.07.03/Makes/Android/jni/jni/Android.mk.
if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
    message(FATAL_ERROR "The official EntisGLS source build currently supports arm64-v8a")
endif()
if(NOT STUDYSTEADY_ROOT)
    get_filename_component(STUDYSTEADY_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()
set(ENTIS_LOQUATY_ROOT "${STUDYSTEADY_ROOT}/vendor/official-loquaty")
set(ENTIS_TINYGLTF_ROOT "${STUDYSTEADY_ROOT}/vendor/official-tinygltf")
foreach(required
        "${ENTIS_ROOT}/Source/common/sakura/ssys_environment.cpp"
        "${ENTIS_LOQUATY_ROOT}/Loquaty/include/loquaty.h"
        "${ENTIS_TINYGLTF_ROOT}/tiny_gltf.h")
    if(NOT EXISTS "${required}")
        message(FATAL_ERROR "Missing official source dependency: ${required}")
    endif()
endforeach()
set(ENTIS_SOURCE_GROUPS
    common/esl android/esl common/sakura android/sakura
    common/sakuracl/erisa common/sakuragl opengl/sakuragl android/sakuragl
    common/sakuragl/erisa common/sakuragl/media common/sakuragl/sgl2d
    common/sakuragl/sgl3d common/sakuragl/window common/sakuraglx
    common/sakuraglx/ui common/sakuraglx/sprite common/sakuraglx/render
    common/sakuraglx/extra common/glscs common/rosetta common/loquaty
    common/antirrhinum)
set(ENTIS_OFFICIAL_SOURCES)
foreach(group IN LISTS ENTIS_SOURCE_GROUPS)
    file(GLOB group_sources CONFIGURE_DEPENDS "${ENTIS_ROOT}/Source/${group}/*.cpp")
    list(APPEND ENTIS_OFFICIAL_SOURCES ${group_sources})
endforeach()
# ndk-build scripts rename these to .cpp.neon; CMake compiles the supplied .cpp.
set(ENTIS_NEON_SOURCES
    "${ENTIS_ROOT}/Source/common/sakuragl/sgl3d/neon/sgl3d_matrix_neon.cpp"
    "${ENTIS_ROOT}/Source/common/sakuraglx/render/neon/sglx3d_collision_neon.cpp")
set_source_files_properties(${ENTIS_NEON_SOURCES} PROPERTIES LANGUAGE CXX)
add_library(gls4 STATIC ${ENTIS_OFFICIAL_SOURCES} ${ENTIS_NEON_SOURCES})
target_include_directories(gls4 PUBLIC
    "${ENTIS_ROOT}/Include/common" "${ENTIS_ROOT}/Include/unix"
    "${ENTIS_ROOT}/Include/opengl" "${ENTIS_ROOT}/Include/android"
    "${ENTIS_ROOT}/Include/android/gls4jclass"
    "${ENTIS_LOQUATY_ROOT}/Loquaty/include"
    PRIVATE "${ENTIS_TINYGLTF_ROOT}" "${ANDROID_NDK}/sources/android/cpufeatures")
target_compile_definitions(gls4 PRIVATE PLATFORM_ANDROID ANDROID_NDK_VER=27 ANDROID_API_LEVEL=29 __arm__=1)
target_compile_options(gls4 PRIVATE -Wno-invalid-offsetof -Wno-deprecated-declarations -fexceptions -frtti -fno-delete-null-pointer-checks)
file(GLOB ENTIS_LOQUATY_SOURCES CONFIGURE_DEPENDS "${ENTIS_LOQUATY_ROOT}/Loquaty/source/*.cpp")
add_library(loquaty STATIC ${ENTIS_LOQUATY_SOURCES})
target_include_directories(loquaty PUBLIC "${ENTIS_LOQUATY_ROOT}/Loquaty/include")
target_compile_options(loquaty PRIVATE -fexceptions -frtti -Wno-invalid-offsetof)
set_target_properties(gls4 loquaty PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(gls4 PUBLIC loquaty)
