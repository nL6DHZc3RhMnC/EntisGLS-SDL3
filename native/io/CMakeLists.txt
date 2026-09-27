# Core storage routing has no SDL, JNI, UIKit or SDK dependency.
add_library(entis_game_files STATIC game_files.cpp)
set_target_properties(entis_game_files PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(entis_game_files PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
