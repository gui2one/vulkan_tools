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
target_link_libraries(vulkan_tools PRIVATE vulkan-1.lib)


add_executable(${PROJECT_NAME} 
    src/main.cpp
)

add_subdirectory(vendor/glad)
target_link_libraries(${PROJECT_NAME} PUBLIC glad)

set(_include_dirs 
    ./src
    vendor/glad/include
    ${VULKAN_SDK}/Include/
    ${CMAKE_SOURCE_DIR}/build/_deps/glfw-src/include
    ${CMAKE_SOURCE_DIR}/build/_deps/glm-src/
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/misc/cpp
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/backends
    ${CMAKE_SOURCE_DIR}/build/_deps/imgui_node_editor-src    
)

include_directories(${_include_dirs})
target_include_directories(${PROJECT_NAME} PUBLIC ${_include_dirs})
target_link_libraries(${PROJECT_NAME} PRIVATE
    vulkan_tools
    imgui
    glfw)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<IF:$<CONFIG:Debug>,Debug,Release>)
if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/compiled_shaders
        $<IF:$<CONFIG:Debug>,${CMAKE_CURRENT_BINARY_DIR}/Debug/compiled_shaders,${CMAKE_CURRENT_BINARY_DIR}/Release/compiled_shaders>
    COMMENT "Copying compiled shaders into binary dir"    
)
