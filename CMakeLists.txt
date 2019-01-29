# This is the root LunaPaak build file.
# It lists all the libraries and then uses them to build the main executable

cmake_minimum_required(VERSION 3.0)
project(LunaPaak C CXX)

# TODO: Check if libraries are present (as submodules)

add_definitions(-DLUA_COMPAT_MODULE)
add_subdirectory (lib EXCLUDE_FROM_ALL)
add_subdirectory (src)
