cmake_minimum_required(VERSION 3.0.0)
project(LearnVulkan VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
# include glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("third_party/glfw")
set(GLFW_INCLUDE_DIRS "third_party/glfw/include")

# include glm
list(APPEND CMAKE_PREFIX_PATH third_party/glm/cmake)
find_package(glm REQUIRED FATAL_ERROR)

# include vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)

# include boost
find_package(Boost REQUIRED)

# include glog
find_package(glog REQUIRED FATAL_ERROR)

# includ vulkan hpp
if (DEFINED ENV{VULKAN_HPP_DIR})
  set(VULKAN_HPP_INCLUDE_DIR $ENV{VULKAN_HPP_DIR})
else()
  message(FATAL_ERROR  "VULKAN_HPP_DIR NOT SET")
endif()

# third party include and link
list(APPEND LEARN_VULKAN_COMMON_INCLUDE_DIRS ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${VULKAN_HPP_INCLUDE_DIR} ${GLOG_INCLUDE_DIR})
list(APPEND LEANN_VULKAN_COMMON_LINK_LIBRARYIES  Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm glog::glog)


add_executable(LearnVulkanDrawTriangle src/draw_triangle.cpp)
target_include_directories(LearnVulkanDrawTriangle PUBLIC ${LEARN_VULKAN_COMMON_INCLUDE_DIRS} include/)
target_link_libraries(LearnVulkanDrawTriangle ${LEANN_VULKAN_COMMON_LINK_LIBRARYIES} )
