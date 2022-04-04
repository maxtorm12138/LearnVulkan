cmake_minimum_required(VERSION 3.21.1)
project(LearnVulkan VERSION 0.1.0)

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    # this change happened in CMake 3.20.4
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("third_party/glfw")

# include glm
list(APPEND CMAKE_PREFIX_PATH third_party/glm/cmake)
find_package(glm REQUIRED FATAL_ERROR)

# include vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)

# include boost
find_package(Boost REQUIRED COMPONENTS log)

# include vulkan hpp
if (DEFINED ENV{VULKAN_HPP_DIR})
  set(VULKAN_HPP_INCLUDE_DIR $ENV{VULKAN_HPP_DIR})
else()
  message(FATAL_ERROR  "VULKAN_HPP_DIR NOT SET")
endif()

# shader compiler
function(target_add_shader TARGET SHADER)
  set(CURRENT_SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
  set(CURRENT_OUTPUT_PATH ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)
  get_filename_component(CURRENT_OUTPUT_DIR ${CURRENT_OUTPUT_PATH} DIRECTORY)
  file(MAKE_DIRECTORY ${CURRENT_OUTPUT_DIR})

  add_custom_command(
    OUTPUT ${CURRENT_OUTPUT_PATH}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${CURRENT_OUTPUT_PATH} ${CURRENT_SHADER_PATH}
    DEPENDS ${CURRENT_SHADER_PATH}
    IMPLICIT_DEPENDS CXX ${CURRENT_SHADER_PATH}
    VERBATIM)

    set_source_files_properties(${CURRENT_OUTPUT_PATH} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${CURRENT_OUTPUT_PATH})
endfunction(target_add_shader TARGET SHADER)

file(GLOB SRCS src/*.cpp include/*.hpp)
add_executable(LearnVulkanDrawTriangle ${SRCS})
target_add_shader(LearnVulkanDrawTriangle triangle.frag)
target_add_shader(LearnVulkanDrawTriangle triangle.vert)
target_include_directories(LearnVulkanDrawTriangle PUBLIC ${Vulkan_INCLUDE_DIRS} ${VULKAN_HPP_INCLUDE_DIR} include/)
target_link_libraries(LearnVulkanDrawTriangle PRIVATE Vulkan::Vulkan glfw glm::glm Boost::log Boost::boost)

