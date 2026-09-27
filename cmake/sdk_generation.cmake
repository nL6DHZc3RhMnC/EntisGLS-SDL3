# The SDK overlays still run during configure because source selection uses
# their output paths. Track both content and membership of every input tree so
# an SDK/header/generator edit triggers regeneration before incremental builds.
include_guard(GLOBAL)
function(entis_track_sdk_generation)
    get_filename_component(_entis_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    file(GLOB_RECURSE _entis_generator_inputs CONFIGURE_DEPENDS
        "${_entis_root}/tools/sdk/*.py"
        "${_entis_root}/native/compatibility/sdk/*.py"
        "${_entis_root}/native/runtime/cotopha_port/*.h"
        "${_entis_root}/native/runtime/cotopha_port/*.inc"
        "${_entis_root}/native/platform/sdl/*.inc"
        "${_entis_root}/vendor/official-loquaty/Loquaty/source/loquaty_file.cpp"
        "${ENTIS_ROOT}/Include/*.h" "${ENTIS_ROOT}/Source/*.cpp"
        "${LEGACY_ROOT}/*.h" "${LEGACY_ROOT}/*.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${_entis_root}/tools/_bootstrap.py" ${_entis_generator_inputs})
endfunction()
