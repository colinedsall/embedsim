# Taken from https://www.jetbrains.com/help/clion/cmakelists-txt-file.html
# Adapted for Qt6

cmake_minimum_required(VERSION 3.13)  # CMake version check
project(embedsim)                     # Create project "embedsim"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
# Fallback to Qt5 if Qt6 is not available
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()

# Add main.cpp file of the project root directory as a source file
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(embedsim ${SOURCE_FILES})

# Link Qt libraries
if(Qt6_FOUND)
    target_link_libraries(embedsim Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(embedsim Qt5::Core Qt5::Widgets)
endif()

# Enable Qt MOC (Meta-Object Compiler)
set_target_properties(embedsim PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)