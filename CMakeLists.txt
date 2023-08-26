CMAKE_MINIMUM_REQUIRED( VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(PROJECT_NAME "TS_ENGINE_Editor")
project(${PROJECT_NAME})

message("Architecture: " ${CMAKE_ARCHITECTURE})
message("BuildType: " ${CMAKE_BUILD_TYPE})
message("MSVC_VERSION: ${MSVC_VERSION}")

# Set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/lib )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/lib )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/bin )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Add subdirectory
set(TS_ENGINE_DIR ${CMAKE_SOURCE_DIR}/../TS_ENGINE)
set(TS_ENGINE_BINARY_DIR ${CMAKE_BINARY_DIR}/TS_ENGINE)
add_subdirectory(${TS_ENGINE_DIR} ${TS_ENGINE_BINARY_DIR})

#SOURCE_FILES
set (SOURCE_FILES
${TS_ENGINE_DIR}/Dependencies/src/glad/glad.c
src/Sandbox.cpp
)

#ImGui Filter
file(GLOB ImGuiSrc
${TS_ENGINE_DIR}/Dependencies/include/imgui/imconfig.h
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui.h
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui_demo.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui_draw.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/backends/imgui_impl_glfw.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/backends/imgui_impl_glfw.h
${TS_ENGINE_DIR}/Dependencies/include/imgui/backends/imgui_impl_opengl3.h
${TS_ENGINE_DIR}/Dependencies/include/imgui/backends/imgui_impl_opengl3.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui_tables.cpp
${TS_ENGINE_DIR}/Dependencies/include/imgui/imgui_widgets.cpp
${TS_ENGINE_DIR}/Dependencies/include/ImGuizmo/ImGuizmo.h
${TS_ENGINE_DIR}/Dependencies/include/ImGuizmo/ImGuizmo.cpp
)
source_group("ImGui" FILES ${ImGuiSrc})

#Layers Filter
file(GLOB LayersSrc
src/EditorLayer.h
src/EditorLayer.cpp
)

source_group("Layers" FILES ${LayersSrc})

file(GLOB LayersEditorSrc
src/Editor/SceneGui.h
src/Editor/SceneGui.cpp
)

source_group("Layers/Editor" FILES ${LayersEditorSrc})

#Shaders Filter
file(GLOB ShadersSrc
${CMAKE_SOURCE_DIR}/Assets/Shaders/BatchLit.vert
${CMAKE_SOURCE_DIR}/Assets/Shaders/BatchLit.frag
${CMAKE_SOURCE_DIR}/Assets/Shaders/Lit.vert
${CMAKE_SOURCE_DIR}/Assets/Shaders/Lit.frag
${CMAKE_SOURCE_DIR}/Assets/Shaders/HDRLighting.vert
${CMAKE_SOURCE_DIR}/Assets/Shaders/HDRLighting.frag
${CMAKE_SOURCE_DIR}/Assets/Shaders/PickingLit.frag
)
source_group("Shaders" FILES ${ShadersSrc})

# Add an additional library directory
link_directories(
${TS_ENGINE_DIR}/Dependencies/${CMAKE_ARCHITECTURE}-windows/debug/lib
${TS_ENGINE_DIR}/Dependencies/include/assimp/build/${CMAKE_ARCHITECTURE}/lib/${CMAKE_BUILD_TYPE}
)

add_executable (${PROJECT_NAME} 
${SOURCE_FILES} # SOURCE_FILES Default Filter
${LayersSrc} #Layers Filter
${LayersEditorSrc} #Layers/Editor Filter
${ShadersSrc} #Shaders Filter
${ImGuiSrc} #ImGui Filter
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Assets
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/bin/Assets
COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/bin/Assets)

#Add definations
add_compile_definitions(
    _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
    WIN32
    _DEBUG
    _WIN64
    BT_USE_DOUBLE_PRECISION
    USE_GRAPHICAL_BENCHMARK
    _CRT_SECURE_NO_WARNINGS
    _CRT_SECURE_NO_DEPRECATE
    _SCL_SECURE_NO_WARNINGS
    TS_PLATFORM_WINDOWS
	TS_ENGINE_EDITOR
	IMGUI_DEFINE_MATH_OPERATORS
    )

#Include directories
target_include_directories (${PROJECT_NAME} PUBLIC 
${CMAKE_SOURCE_DIR}/src
)

#Add libraries
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC TS_ENGINE
opengl32.lib
glfw3.lib
)

if(MSVC_VERSION EQUAL 1929)
    message("Visual Studio 2019 detected")
	target_link_libraries(${PROJECT_NAME} LINK_PUBLIC TS_ENGINE assimp-vc142-mtd.lib)
elseif(MSVC_VERSION GREATER_EQUAL 1920 AND MSVC_VERSION LESS_EQUAL 1928)
    message("Visual Studio 2019 detected (with patch version)")
elseif(MSVC_VERSION GREATER_EQUAL 1934 AND MSVC_VERSION LESS_EQUAL 2031)
    message("Visual Studio 2022 detected")
	target_link_libraries(${PROJECT_NAME} LINK_PUBLIC TS_ENGINE assimp-vc143-mtd.lib)
else()
    message("Unsupported Visual Studio version")
endif()
