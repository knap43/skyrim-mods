cmake_minimum_required(VERSION 3.21)

project(SkyrimMods LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Every mod here targets Skyrim SE 1.5.97 only.
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/CommonLibSSE-NG EXCLUDE_FROM_ALL)
include(extern/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

# Each mod lives in mods/<Name>; a mod with a CMakeLists.txt is an SKSE plugin.
# Its dist/ folder is copied verbatim into the packaged Data/ folder.
set(SKYRIM_MODS_PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")

function(skyrim_mod_package TARGET MOD_DIR)
    # CommonLib headers (and its generated plugin-info stub) expect its PCH first.
    target_precompile_headers("${TARGET}" PRIVATE "${CMAKE_SOURCE_DIR}/cmake/PCH.h")

    set(out "${SKYRIM_MODS_PACKAGE_DIR}/${TARGET}")
    add_custom_command(
        TARGET "${TARGET}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${out}/SKSE/Plugins"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${MOD_DIR}/dist" "${out}"
        COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${TARGET}>" "${out}/SKSE/Plugins/"
        VERBATIM)
endfunction()

file(GLOB mod_dirs LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/mods/*")
foreach(dir IN LISTS mod_dirs)
    if(EXISTS "${dir}/CMakeLists.txt")
        add_subdirectory("${dir}")
    endif()
endforeach()
