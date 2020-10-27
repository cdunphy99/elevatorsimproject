cmake_minimum_required(VERSION 3.16)
project(elevatorsimproject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -pthread)
add_executable(elevatorsimproject main.c dataStructures.h randomGeneration.h)