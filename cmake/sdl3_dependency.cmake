# Optional platform dependency. Including this file defines SDL3::SDL3 from the
# pinned source distribution; it does not change the existing Android backend.
include_guard(GLOBAL)

get_filename_component(STUDYSTEADY_SDL3_ROOT "${CMAKE_CURRENT_LIST_DIR}/../vendor/sdl3" ABSOLUTE)
if(NOT EXISTS "${STUDYSTEADY_SDL3_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "SDL3 sources are missing. Run: python3 tools/setup_sdl3.py")
endif()

# SDL's Android Java bootstrap loads libSDL3.so. Other platforms can link the
# dependency statically without requiring a separately installed SDL runtime.
option(STUDYSTEADY_SDL3_SHARED "Build shared SDL3 (required by the Android SDL bootstrap)" ${ANDROID})
if(STUDYSTEADY_SDL3_SHARED)
    set(SDL_SHARED ON CACHE BOOL "Build SDL3 shared library" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "Build SDL3 static library" FORCE)
else()
    set(SDL_SHARED OFF CACHE BOOL "Build SDL3 shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL3 static library" FORCE)
endif()
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Do not build SDL3 test helpers" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Do not build SDL3 tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Do not build SDL3 examples" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "Do not install vendored SDL3" FORCE)
set(SDL_INSTALL_TESTS OFF CACHE BOOL "Do not install SDL3 tests" FORCE)
set(SDL_UNINSTALL OFF CACHE BOOL "Do not create an SDL3 uninstall target" FORCE)
add_subdirectory("${STUDYSTEADY_SDL3_ROOT}" "${CMAKE_BINARY_DIR}/sdl3")
