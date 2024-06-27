cmake_minimum_required(VERSION 3.28)

project(Quakernel LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_custom_command(
    COMMAND make CC=${CMAKE_C_COMPILER}
    COMMAND ${CMAKE_COMMAND} -E touch limine_make.stamp
    OUTPUT limine_make.stamp
    )