# Read the traditional sources directly from the official SDK. Generated
# compatibility copies remain in the build directory; originals stay unchanged.
include("${CMAKE_CURRENT_LIST_DIR}/entis_sdk.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sdk_generation.cmake")
entis_track_sdk_generation()
set(LEGACY_PORT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../native/compatibility/sdk/legacy")
set(LEGACY_RUNTIME_ROOT "${CMAKE_CURRENT_LIST_DIR}/../native/runtime/cotopha_port")
set(LEGACY_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/legacy_generated")
file(GLOB LEGACY_PATCH_TOOLS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../tools/sdk/patch_legacy_*.py")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../tools/sdk/prepare_cotopha.py" ${LEGACY_PATCH_TOOLS})
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/../tools/sdk/prepare_cotopha.py"
    --legacy "${LEGACY_ROOT}" --output "${LEGACY_GENERATED}"
    COMMAND_ERROR_IS_FATAL ANY)
add_library(legacy_foundation STATIC
    "${LEGACY_GENERATED}/ESL/Source/eslarray.cpp"
    "${LEGACY_GENERATED}/ESL/Source/eslstring.cpp"
    "${LEGACY_GENERATED}/ESL/Source/esldesc.cpp"
    "${LEGACY_RUNTIME_ROOT}/legacy_file.cpp"
    "${LEGACY_PORT_ROOT}/platform.cpp")
set_target_properties(legacy_foundation PROPERTIES POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
if(STUDYSTEADY_SDL3)
    target_link_libraries(legacy_foundation PUBLIC study_sdl_platform)
endif()
target_include_directories(legacy_foundation PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/../native"
    "${LEGACY_PORT_ROOT}" "${LEGACY_GENERATED}/ESL/Include"
    "${LEGACY_GENERATED}/GLS3/Include")
if(STUDYSTEADY_SDL3)
    target_include_directories(legacy_foundation PUBLIC ${SDL_SDK_INCLUDES})
endif()
target_include_directories(legacy_foundation PUBLIC
    "${ENTIS_ROOT}/Include/common" "${ENTIS_ROOT}/Include/unix"
    "${ENTIS_ROOT}/Include/opengl")
target_compile_definitions(legacy_foundation PUBLIC _DISABLE_ESL_NEW=1)
if(NOT STUDYSTEADY_SDL3)
    target_include_directories(legacy_foundation PUBLIC "${ENTIS_ROOT}/Include/android")
    target_compile_definitions(legacy_foundation PUBLIC PLATFORM_ANDROID=1
        ANDROID_NDK_VER=27 ANDROID_API_LEVEL=29 __arm__=1)
endif()
target_compile_options(legacy_foundation PRIVATE -Wno-invalid-source-encoding
    -Wno-deprecated-declarations -Wno-invalid-offsetof -Wno-tautological-undefined-compare)

option(LEGACY_COMPILE_OBJECTS "Compile the GLS3 object types (not a complete linked runtime yet)" ON)
if(LEGACY_COMPILE_OBJECTS)
    set(LEGACY_OBJECT_NAMES object reference pointer integer real string array hash
        structure buffer buffer_structure stack global function)
    set(LEGACY_OBJECT_SOURCES)
    foreach(name IN LISTS LEGACY_OBJECT_NAMES)
        list(APPEND LEGACY_OBJECT_SOURCES "${LEGACY_GENERATED}/GLS3/Source/glscsobj_${name}.cpp")
    endforeach()
    add_library(legacy_objects STATIC ${LEGACY_OBJECT_SOURCES}
        "${LEGACY_PORT_ROOT}/runtime_support.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_environment.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_thread.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_script_file.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_atomic_path.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_atomic_file.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_resource.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_image_export.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_resource_manager.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_input.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_window.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_setup.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_message.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_emote.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_audio_player.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_movie.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_compiler.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_sprite.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_sprite_dynamic.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_super_sprite.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_super_raster.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_super_shading.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_super_shading_math.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_particle.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_particle_model.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_tone_filter.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_sprite_draw.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_sprite_callbacks.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_motion_graphics.cpp"
        "${LEGACY_RUNTIME_ROOT}/legacy_native_binding.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/legacy_emc.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/legacy_script_types.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_compiler.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_assembler.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_execution_image_compiler.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_execution_image_linker.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_rosetta.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_context.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_context_naked.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_execution_image.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glsscriptobj.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_classinf.cpp"
        "${LEGACY_GENERATED}/GLS3/Source/glscs_inter_file.cpp")
    target_link_libraries(legacy_objects PUBLIC legacy_foundation)
    target_compile_options(legacy_objects PRIVATE -fno-delete-null-pointer-checks
        -Wno-tautological-undefined-compare -Wno-invalid-source-encoding
        -Wno-invalid-offsetof -Wno-deprecated-declarations)
    target_link_libraries(legacy_objects PUBLIC entis_psb_keys)
    set_target_properties(legacy_objects PROPERTIES POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
endif()
