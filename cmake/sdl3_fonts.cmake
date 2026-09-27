# OTF/CFF font rendering is shared by every SDL target; no system font install.
include_guard(GLOBAL)
set(STUDY_FREETYPE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype")
if(NOT EXISTS "${STUDY_FREETYPE_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "Font dependencies are missing. Run python3 tools/sdk/setup_sdl_fonts.py")
endif()
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/sdk/setup_sdl_fonts.py" --verify --freetype-only
    COMMAND_ERROR_IS_FATAL ANY)
foreach(dependency ZLIB BZIP2 PNG HARFBUZZ BROTLI)
    set(FT_DISABLE_${dependency} ON CACHE BOOL "Use the self-contained game OTF rasterizer" FORCE)
endforeach()
function(study_add_freetype)
    # Scope these upstream options so SDL and other libraries keep their own policy.
    set(BUILD_SHARED_LIBS OFF)
    set(SKIP_INSTALL_ALL ON)
    add_subdirectory("${STUDY_FREETYPE_ROOT}" "${CMAKE_BINARY_DIR}/freetype" EXCLUDE_FROM_ALL)
endfunction()
study_add_freetype()
set_target_properties(freetype PROPERTIES POSITION_INDEPENDENT_CODE ON)
