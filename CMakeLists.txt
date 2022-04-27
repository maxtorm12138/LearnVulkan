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
message("Vulkan: ${Vulkan_INCLUDE_DIRS}")

# include boost
find_package(Boost REQUIRED COMPONENTS log)

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
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan1.1 -o ${CURRENT_OUTPUT_PATH} ${CURRENT_SHADER_PATH}
            DEPENDS ${CURRENT_SHADER_PATH}
            IMPLICIT_DEPENDS CXX ${CURRENT_SHADER_PATH}
            VERBATIM)

    set_source_files_properties(${CURRENT_OUTPUT_PATH} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${CURRENT_OUTPUT_PATH})
endfunction(target_add_shader TARGET SHADER)

add_library(vma include/vma/vk_mem_alloc.h src/vma/vk_mem_alloc.cpp)
target_include_directories(vma PRIVATE ${Vulkan_INCLUDE_DIRS} include/)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(vma PRIVATE -Wno-nullability-completeness)
endif ()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vk_layer_settings.txt DESTINATION ${CMAKE_BINARY_DIR})

add_library(stb include/stb/stb_image.h src/stb/stb_image.cpp)
target_include_directories(stb PRIVATE include/)

file(GLOB LVK_SRCS include/lvk_*.hpp src/lvk_*.cpp)
add_library(lvk ${LVK_SRCS})
target_add_shader(lvk naive/naive.frag)
target_add_shader(lvk naive/naive.vert)
target_compile_definitions(lvk PRIVATE -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS -DVULKAN_HPP_NO_SPACESHIP_OPERATOR -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE -DGLM_FORCE_LEFT_HANDED)
target_include_directories(lvk PRIVATE ${Vulkan_INCLUDE_DIRS} ${SDL2PP_INCLUDE_DIRS} include/)
target_link_libraries(lvk PRIVATE ${SDL2PP_LIBRARIES} vma stb Vulkan::Vulkan glm::glm Boost::log Boost::boost fmt::fmt-header-only)

add_executable(engine src/main.cpp)
target_include_directories(engine PRIVATE include/)
target_link_libraries(engine PRIVATE lvk Boost::log)
