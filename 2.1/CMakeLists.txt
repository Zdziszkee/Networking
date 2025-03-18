# Set the minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Name your project
project(file_operations C)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-pedantic -Wall -Wextra)
set(CMAKE_VERBOSE_MAKEFILE ON)
# Add the second executable (named "server")
add_executable(server server.c)

# Add the second executable (named "server")
add_executable(client client.c)
