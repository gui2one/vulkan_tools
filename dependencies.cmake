include(FetchContent)   

set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)
set(FETCHCONTENT_QUIET OFF)


function(FetchDependencyAndMakeAvailable NAME URL TAG)
  FetchContent_Declare(
    ${NAME}
    GIT_REPOSITORY ${URL}
    GIT_TAG        ${TAG}
    GIT_PROGRESS TRUE
  )
  FetchContent_MakeAvailable(${NAME})
endfunction()

# GLM
FetchDependencyAndMakeAvailable(
  glm
  https://github.com/g-truc/glm
  33b4a621a697a305bc3a7610d290677b96beb181
)

# GLFW
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_SHARED_LIBRARY ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchDependencyAndMakeAvailable(
  glfw
  https://github.com/glfw/glfw
  7b6aead9fb88b3623e3b3725ebb42670cbe4c579
)

#IMGUI
FetchDependencyAndMakeAvailable(
  imgui
  https://github.com/ocornut/imgui
  v1.91.5-docking
)

set(imgui_sources 
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/backends/imgui_impl_glfw.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/backends/imgui_impl_opengl3.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/misc/cpp/imgui_stdlib.cpp

  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/imgui.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/imgui_demo.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/imgui_draw.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/imgui_widgets.cpp
  ${CMAKE_SOURCE_DIR}/build/_deps/imgui-src/imgui_tables.cpp     
)

add_library(imgui STATIC ${imgui_sources})

target_include_directories(imgui PUBLIC 
  ${CMAKE_SOURCE_DIR}/_deps/imgui-src
  ${CMAKE_SOURCE_DIR}/_deps/glfw-src/include
)