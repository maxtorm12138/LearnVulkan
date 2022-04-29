cmake_minimum_required(VERSION 3.21.0)
project(VulkanHpp VERSION 3.0.1)

find_package(Vulkan REQUIRED FATAL_ERROR)

add_library(vulkan_hpp INTERFACE)
target_include_directories(vulkan_hpp INTERFACE include/)
target_link_libraries(vulkan_hpp INTERFACE Vulkan::Vulkan)
add_library(vulkan::vulkancpp ALIAS vulkan_hpp)