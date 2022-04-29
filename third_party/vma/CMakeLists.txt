cmake_minimum_required(VERSION 3.21.0)
project(vma VERSION 3.0.1)

find_package(Vulkan REQUIRED FATAL_ERROR)

# vma static
add_library(vma STATIC vk_mem_alloc.h vk_mem_alloc.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(vma PRIVATE -Wno-nullability-completeness)
endif ()

target_link_libraries(vma PUBLIC Vulkan::Vulkan)
target_include_directories(vma PUBLIC ./)
add_library(vma::vma ALIAS vma)