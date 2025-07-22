# Taken from https://www.jetbrains.com/help/clion/cmakelists-txt-file.html

cmake_minimum_required(VERSION 3.13)  # CMake version check
project(simple_example)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Add main.cpp file of the project root directory as a source file
set(SOURCE_FILES main.cpp clock.cpp timer.cpp io.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(main ${SOURCE_FILES})