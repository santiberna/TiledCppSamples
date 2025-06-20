add_executable(AdvancedWarsSample)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "*.cpp" "*.hpp")
target_sources(AdvancedWarsSample
    PRIVATE
        ${sources}
)

target_include_directories(AdvancedWarsSample
    PRIVATE ""
)

target_link_libraries(AdvancedWarsSample
    PRIVATE
        Framework2D
        TiledCpp
)