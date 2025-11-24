cmake_minimum_required(VERSION 3.16)
project(Bil_simulator_John_Mitchel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)

FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG 619cdaacc2e7dc6ea709d8c7f482e2994af7349d
)
FetchContent_MakeAvailable(threepp)


FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)


add_library(bilsim_core
        src/Game.cpp
        src/World.cpp
        src/Car.cpp
        src/GameObject.cpp
        src/Pickup.cpp
        src/Obstacle.cpp
)

target_include_directories(bilsim_core PUBLIC src)


# Fix MSVC "out of heap space" error
if (MSVC)
    add_compile_options(/bigobj)
    add_compile_options(/Zm200)  # increase compiler heap
endif()



# ------------------------
# Main Executable
# ------------------------
add_executable(Bil_simulator_John_Mitchel
        src/main.cpp

)

target_link_libraries(Bil_simulator_John_Mitchel
        PRIVATE
        bilsim_core
        threepp

)


add_executable(bilsim_tests
        tests/test_car.cpp
)

target_link_libraries(bilsim_tests
        PRIVATE
        bilsim_core
        Catch2::Catch2WithMain
)
