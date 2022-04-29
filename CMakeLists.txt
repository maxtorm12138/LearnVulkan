cmake_minimum_required(VERSION 3.21.0)
project(LearnVulkan VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(BUILD_SHARED_LIBS TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# third party sources
add_subdirectory("third_party/vma")
add_subdirectory("third_party/glm")
add_subdirectory("third_party/fmt")
add_subdirectory("third_party/stb")
add_subdirectory("third_party/vulkan")

# include sdl2
set(SDL2_BUILDING_LIBRARY ON)
find_package(SDL2 REQUIRED)
message("SDL2 ${SDL2_LIBRARIES}")

# include boost
find_package(Boost REQUIRED COMPONENTS log)

# sdl2 cpp wrapper
add_library(sdl2pp STATIC src/sdl2pp/sdl2pp.hpp src/sdl2pp/sdl2pp.cpp)
target_link_libraries(sdl2pp PUBLIC SDL2::Core vulkan::vulkancpp)
target_link_libraries(sdl2pp PRIVATE Boost::boost fmt::fmt-header-only)

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

file(GLOB LVK_SRCS src/lvk/lvk_*.hpp src/lvk/lvk_*.cpp)
add_library(lvk SHARED ${LVK_SRCS})
target_add_shader(lvk naive/naive.frag)
target_add_shader(lvk naive/naive.vert)
target_compile_definitions(lvk PRIVATE -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS -DVULKAN_HPP_NO_SPACESHIP_OPERATOR -DGLM_FORCE_RADIANS -DGLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(lvk PRIVATE src/)
target_link_libraries(lvk PRIVATE vma::vma vulkan::vulkancpp sdl2pp glm::glm Boost::log Boost::boost fmt::fmt-header-only)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vk_layer_settings.txt DESTINATION ${CMAKE_BINARY_DIR})
add_executable(engine src/main.cpp)
target_include_directories(engine PRIVATE src/)
target_link_libraries(engine lvk)
target_link_libraries(engine Boost::log)
