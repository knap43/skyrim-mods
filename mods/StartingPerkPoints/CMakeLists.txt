add_commonlibsse_plugin(StartingPerkPoints
    NAME "StartingPerkPoints"
    AUTHOR "knap43"
    VERSION 1.0.0
    USE_ADDRESS_LIBRARY
    SOURCES src/main.cpp)

skyrim_mod_package(StartingPerkPoints "${CMAKE_CURRENT_SOURCE_DIR}")
