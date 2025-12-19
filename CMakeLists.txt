cmake_minimum_required(VERSION 3.10)
project(CarServerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(Boost REQUIRED COMPONENTS system)

include_directories(include)

add_executable(server main.cpp)
target_link_libraries(server Boost::system)

add_executable(tests tests.cpp)


enable_testing()
add_test(NAME CarServerTests COMMAND tests)
