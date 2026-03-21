cmake_minimum_required(VERSION 3.10)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(raytracer ${SOURCES})

# Include directories
target_include_directories(raytracer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optional: Add compiler flags for optimization
if(MSVC)
    target_compile_options(raytracer PRIVATE /W4)
else()
    target_compile_options(raytracer PRIVATE -Wall -Wextra -Wpedantic)
endif()