### PROJECT
cmake_minimum_required(VERSION 3.19.6)
project(TiledCppSamples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## DEPENDENCIES

include(FetchContent)

FetchContent_Declare(
    framework2d
    GIT_REPOSITORY https://github.com/santiberna/Framework2D.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(framework2d)

FetchContent_Declare(
    tiledcpp
    GIT_REPOSITORY https://github.com/santiberna/TiledCPP.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(tiledcpp)

add_subdirectory(tactical_wars_sample)