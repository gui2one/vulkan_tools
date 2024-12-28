cmake_minimum_required(VERSION 3.20)

include(${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake)
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "Vulkan SDK not found. Please set VULKAN_SDK environment variable")
else()
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    message(STATUS "Vulkan SDK found at $ENV{VULKAN_SDK}")
endif()

project(vulkan_renderer LANGUAGES CXX)


add_library(vulkan_tools STATIC 
    src/vulkan_tools/vulkan_tools.cpp
)

target_include_directories(vulkan_tools PUBLIC 
    src/vulkan_tools
    ${VULKAN_SDK}/Include/
)
target_link_directories(vulkan_tools PUBLIC ${VULKAN_SDK}/Lib)
target_link_libraries(vulkan_tools PRIVATE vulkan-1.lib )


add_executable(${PROJECT_NAME} 
    src/main.cpp
)

add_subdirectory(vendor/glad)
target_link_libraries(${PROJECT_NAME} PUBLIC glad)

set(_include_dirs 
    vendor/glad/include
    ${VULKAN_SDK}/Include/
    ${CMAKE_SOURCE_DIR}/build/_deps/glfw-src/include
    ${CMAKE_SOURCE_DIR}/build/_deps/glm-src/
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/backends
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui_node_editor-src    
)

include_directories(${_include_dirs})
target_include_directories(${PROJECT_NAME} PUBLIC ${_include_dirs})
target_link_libraries(${PROJECT_NAME} PRIVATE
    vulkan_tools
    imgui
    glfw)
