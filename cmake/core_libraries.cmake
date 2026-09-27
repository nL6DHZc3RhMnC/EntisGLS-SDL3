# Shared storage and E-mote parameter services, independent of the platform bootstrap.
add_library(entis_psb_keys STATIC native/extensions/emote/psb/psb_key_resolver.cpp
    native/extensions/emote/psb/psb_key_runtime.cpp native/launcher/psb_key_settings.cpp)
set_target_properties(entis_psb_keys PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(entis_psb_keys PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/native")
target_link_libraries(entis_psb_keys PUBLIC z entis_game_files)
