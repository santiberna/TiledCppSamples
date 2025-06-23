add_executable(TacticalWarsSample)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp" "*.hpp")
target_sources(TacticalWarsSample
    PRIVATE
        ${sources}
)

target_include_directories(TacticalWarsSample
    PRIVATE ""
)

target_link_libraries(TacticalWarsSample
    PRIVATE
        Framework2D
        TiledCpp
)