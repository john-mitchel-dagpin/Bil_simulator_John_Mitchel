cmake_minimum_required(VERSION 3.16)
project(Bil_simulator_John_Mitchel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


FetchContent_Declare(
        threepp
        GIT_REPOSITORY https://github.com/markaren/threepp.git
        GIT_TAG 619cdaacc2e7dc6ea709d8c7f482e2994af7349d
)

set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)

FetchContent_MakeAvailable(threepp)


FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
)

FetchContent_MakeAvailable(catch2)

# ---- main executable ----
add_executable(bilsim

        src/main.cpp
        src/Car.cpp
        src/Game.cpp
)

# include our source folder
target_include_directories(bilsim PUBLIC src)

# include threepp headers
target_include_directories(bilsim PUBLIC ${threepp_SOURCE_DIR}/include)

target_link_libraries(bilsim PUBLIC threepp::threepp)

#---- tests ----
add_executable(test_physics
        tests/test_physics.cpp
        src/Car.cpp
)

target_include_directories(test_physics PUBLIC src)
target_link_libraries(test_physics PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_physics)
