cmake_minimum_required(VERSION 3.23.0)
project(LearnVulkan VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
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
find_package(Boost REQUIRED COMPONENTS log)

# includ vulkan hpp
if (DEFINED ENV{VULKAN_HPP_DIR})
  set(VULKAN_HPP_INCLUDE_DIR $ENV{VULKAN_HPP_DIR})
else()
  message(FATAL_ERROR  "VULKAN_HPP_DIR NOT SET")
endif()

# third party include and link
list(APPEND LEARN_VULKAN_COMMON_INCLUDE_DIRS ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS}  ${VULKAN_HPP_INCLUDE_DIR})
list(APPEND LEANN_VULKAN_COMMON_LINK_LIBRARYIES  Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glm::glm Boost::log Boost::boost)

# shader files
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

message("${CMAKE_BINARY_DIR}")

add_executable(LearnVulkanDrawTriangle src/draw_triangle.cpp)
target_add_shader(LearnVulkanDrawTriangle triangle.frag)
target_add_shader(LearnVulkanDrawTriangle triangle.vert)
target_include_directories(LearnVulkanDrawTriangle PUBLIC ${LEARN_VULKAN_COMMON_INCLUDE_DIRS} include/)
target_link_libraries(LearnVulkanDrawTriangle ${LEANN_VULKAN_COMMON_LINK_LIBRARYIES})

