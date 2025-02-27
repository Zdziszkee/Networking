# Set the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Name your project
project(1 C)

# Specify the C standard (optional, but good practice)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the executable target
add_executable(1 main.c)
