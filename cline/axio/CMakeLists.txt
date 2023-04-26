cmake_minimum_required(VERSION 3.10...3.21)

project(cline)

# use -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)
# support folders in IDE's
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
