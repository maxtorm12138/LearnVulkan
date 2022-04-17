cmake_minimum_required(VERSION 3.21.0)
project(LearnVulkan VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include sdl2pp
find_package(SDL2PP REQUIRED)

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
else ()
    message(FATAL_ERROR "VULKAN_HPP_DIR NOT SET")
endif ()

# vulkan memory allocator
add_subdirectory("third_party/VulkanMemoryAllocator")

# fmt lib
add_subdirectory("third_party/fmt")

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

file(GLOB LVK_SRCS include/lvk_*.hpp include/VulkanMemoryAllocatorHpp/*.hpp include/VulkanMemoryAllocatorHpp/*.h src/lvk_*.cpp)
add_library(lvk ${LVK_SRCS})
target_compile_definitions(lvk PRIVATE -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS -DVULKAN_HPP_NO_SPACESHIP_OPERATOR -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
target_add_shader(lvk triangle.frag)
target_add_shader(lvk triangle.vert)
target_include_directories(lvk PRIVATE ${Vulkan_INCLUDE_DIRS} ${VULKAN_HPP_INCLUDE_DIR} ${SDL2PP_INCLUDE_DIRS} include/ include/VulkanMemoryAllocatorHpp)
target_link_libraries(lvk PRIVATE ${SDL2PP_LIBRARIES} Vulkan::Vulkan glm::glm Boost::log Boost::boost fmt::fmt-header-only)

add_executable(draw_triangle src/draw_triangle.cpp)
target_include_directories(draw_triangle PRIVATE include/)
target_link_libraries(draw_triangle PRIVATE lvk Boost::log)