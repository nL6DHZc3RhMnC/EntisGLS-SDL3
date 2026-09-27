# All engine inputs come from the supplied official distribution. Force these
# cache entries so an existing build cannot silently retain the previous SDK.
get_filename_component(STUDYSTEADY_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(ENTIS_PACKAGE_ROOT "${STUDYSTEADY_ROOT}/EntisGLS/EntisGLS4.07.03")
set(ENTIS_ROOT "${ENTIS_PACKAGE_ROOT}/Cotopha" CACHE PATH "Official EntisGLS sources" FORCE)
set(LEGACY_ROOT "${ENTIS_PACKAGE_ROOT}/EntisGLS3" CACHE PATH "Official traditional Cotopha sources" FORCE)
foreach(required
    "${ENTIS_ROOT}/Include/common/sakura/ssys_file.h"
    "${LEGACY_ROOT}/GLS3/Source/glscs_context.cpp")
    if(NOT EXISTS "${required}")
        message(FATAL_ERROR "Official SDK input is missing: ${required}")
    endif()
endforeach()
