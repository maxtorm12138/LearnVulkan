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
message("Vulkan: ${Vulkan_LIBRARIES}")

# include boost
find_package(Boost REQUIRED)

list(APPEND LEARN_VULKAN_COMMON_INCLUDE_DIRS ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
list(APPEND LEANN_VULKAN_COMMON_LINK_LIBRARYIES  Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm)

add_executable(Vulkan000 000.cpp)
target_include_directories(Vulkan000 PUBLIC ${LEARN_VULKAN_COMMON_INCLUDE_DIRS})
target_link_libraries(Vulkan000 ${LEANN_VULKAN_COMMON_LINK_LIBRARYIES})


add_executable(Vulkan001 001.cpp)
target_include_directories(Vulkan001 PUBLIC ${LEARN_VULKAN_COMMON_INCLUDE_DIRS})
target_link_libraries(Vulkan001 ${LEANN_VULKAN_COMMON_LINK_LIBRARYIES})
