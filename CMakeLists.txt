cmake_minimum_required(VERSION 3.0.0)
project(LearnVulkan VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("third_party/glfw")


list(APPEND CMAKE_PREFIX_PATH third_party/glm/cmake)
set(GLFW_INCLUDE_DIRS "third_party/glfw/include")

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glm REQUIRED FATAL_ERROR)

add_executable(VulcanEnvironmentTest 000.cpp)
target_include_directories(VulcanEnvironmentTest PUBLIC ${GLM_INCLUDE_DIRS})
target_include_directories(VulcanEnvironmentTest PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(VulcanEnvironmentTest PUBLIC ${GLFW_INCLUDE_DIRS})

target_link_libraries(VulcanEnvironmentTest Vulkan::Vulkan)
target_link_libraries(VulcanEnvironmentTest glfw ${GLFW_LIBRARIES})
target_link_libraries(VulcanEnvironmentTest glm::glm)