# Set the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Name your project
project(file_operations C)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the first executable (named "copy_file")
add_executable(copy_file copy_file_contents.c)
add_compile_options(-pedantic -Wall)

# Add the second executable (named "1")
add_executable(1 main.c)
