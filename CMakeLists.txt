cmake_minimum_required(VERSION 3.6)
project(voxeltatics LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
find_package(SDL2 REQUIRED MODULE COMPONENTS main gfx ttf image)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_library(voxelEngine
    src/core/Camera
    src/core/LoaderComponent
    src/core/PerspectiveRenderComponent
    src/core/ResourcePool
    src/core/Scene
    src/core/Shader
    src/core/Texture
    src/core/VoxelModel
    src/util/fileReader
    src/util/stb_image
)
target_include_directories(voxelEngine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/
    PUBLIC ${GLEW_INCLUDE_DIR}
    PUBLIC ${OPENGL_INCLUDE_DIR}
)
target_link_libraries(voxelEngine
    PRIVATE ${GLEW_LIBRARIES}
    PRIVATE ${OPENGL_LIBRARIES}
)
target_compile_definitions(voxelEngine 
    PUBLIC -DGLM_ENABLE_EXPERIMENTAL
)

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui.h
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_internal.h
    external/imgui/imconfig.h
    external/imgui/imstb_rectpack.h
    external/imgui/imstb_textedit.h
    external/imgui/imstb_truetype.h
)

target_include_directories(imgui
    INTERFACE external/imgui/
)

add_executable(elementViewer
    src/tools/elementViewer
)

target_include_directories(elementViewer
    PRIVATE ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(elementViewer
    PRIVATE voxelEngine
    PRIVATE ${SDL2_LIBRARIES}
    PRIVATE imgui
)

add_executable(sceneViewer
    src/tools/sceneViewer
)

target_include_directories(sceneViewer
    PRIVATE ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(sceneViewer
    PRIVATE voxelEngine
    PRIVATE ${SDL2_LIBRARIES}
    PRIVATE imgui
)

if ( CMAKE_COMPILER_IS_GNUCXX )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -std=gnu++1z")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()