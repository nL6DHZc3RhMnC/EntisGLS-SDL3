# Opt in from CI with ENTISGLS_COMPILER_CACHE=ccache. Keep the real compiler
# selected by project()/the toolchain; cache programs are launchers, not compilers.
include_guard(GLOBAL)
set(_entis_compiler_cache "$ENV{ENTISGLS_COMPILER_CACHE}")
string(STRIP "${_entis_compiler_cache}" _entis_compiler_cache)
if(_entis_compiler_cache STREQUAL "")
    return()
endif()
if(NOT _entis_compiler_cache STREQUAL "ccache")
    message(FATAL_ERROR "Unsupported ENTISGLS_COMPILER_CACHE='${_entis_compiler_cache}'; use ccache or leave it unset")
endif()

# A cross-compiled Android/iOS target still uses the host's cache executable.
find_program(ENTISGLS_CCACHE_EXECUTABLE NAMES ccache NO_CMAKE_FIND_ROOT_PATH)
if(NOT ENTISGLS_CCACHE_EXECUTABLE OR NOT EXISTS "${ENTISGLS_CCACHE_EXECUTABLE}")
    message(FATAL_ERROR "ENTISGLS_COMPILER_CACHE=ccache requires ccache on the build host's PATH")
endif()

if(CMAKE_GENERATOR STREQUAL "Xcode")
    if(NOT DEFINED XCODE_VERSION OR XCODE_VERSION VERSION_LESS "16.0")
        message(FATAL_ERROR "ccache with the Xcode generator requires Xcode 16 or newer for C_COMPILER_LAUNCHER")
    endif()
    # Apple's C-family launcher receives the compiler path as argv[1], followed
    # by its original arguments. This covers C, C++, Objective-C and Objective-C++.
    # Do not set CC/CPLUSPLUS or CMAKE_*_COMPILER to ccache, and leave other
    # languages and link tools under their normal toolchain's control.
    # https://developer.apple.com/documentation/xcode/build-settings-reference
    set(CMAKE_XCODE_ATTRIBUTE_C_COMPILER_LAUNCHER "${ENTISGLS_CCACHE_EXECUTABLE}")

    # Use ordinary header dependencies, with no ccache sloppiness/ignored
    # headers. IDE indexing and SDK stat/module caches are unnecessary for CI
    # binaries, and their generated paths otherwise prevent reusable cache hits.
    # SDK_STAT_CACHE_ENABLE is the Swift Build/Xcode setting (not CLANG_*).
    # https://github.com/swiftlang/swift-build/blob/main/Sources/SWBCore/Specs/CoreBuildSystem.xcspec
    # https://ccache.dev/manual/latest.html#_c_modules
    set(CMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE NO)
    set(CMAKE_XCODE_ATTRIBUTE_SDK_STAT_CACHE_ENABLE NO)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_EXPLICIT_MODULES NO)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES NO)
    set(CMAKE_XCODE_ATTRIBUTE_COMPILATION_CACHE_ENABLE_CACHING NO)
    set(CMAKE_XCODE_ATTRIBUTE_USE_HEADERMAP NO)
elseif(CMAKE_GENERATOR MATCHES "Makefiles|Ninja")
    # These variables initialize the corresponding properties on every target,
    # including dependencies added later with add_subdirectory(). ASM is left
    # untouched; ccache only wraps the supported C-family compilation rules.
    # https://cmake.org/cmake/help/latest/prop_tgt/LANG_COMPILER_LAUNCHER.html
    foreach(_entis_language C CXX OBJC OBJCXX)
        set(CMAKE_${_entis_language}_COMPILER_LAUNCHER "${ENTISGLS_CCACHE_EXECUTABLE}")
    endforeach()
else()
    message(FATAL_ERROR "ENTISGLS_COMPILER_CACHE is not supported with generator '${CMAKE_GENERATOR}'")
endif()
message(STATUS "EntisGLS compiler cache: ${ENTISGLS_CCACHE_EXECUTABLE} (${CMAKE_GENERATOR})")
