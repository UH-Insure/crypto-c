cmake_minimum_required(VERSION 3.16)
project(crypto_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER cc)

include_directories(include)
file(GLOB SRC "src/*.c")
add_executable(crypto_demo ${SRC})
