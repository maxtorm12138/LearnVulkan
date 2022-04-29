cmake_minimum_required(VERSION 3.21.0)
project(stb VERSION 2.27)

# stb static
add_library(stb STATIC stb_image.h stb_image.cpp)
add_library(stb::stb ALIAS stb)