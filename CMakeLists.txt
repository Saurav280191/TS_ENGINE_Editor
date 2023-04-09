CMAKE_MINIMUM_REQUIRED( VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
project(TS_BatchingDemo)
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message("Architecture: " ${CMAKE_ARCHITECTURE})
message("BuildType: " ${CMAKE_BUILD_TYPE})

# Set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/lib )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/lib )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/build/${CMAKE_ARCHITECTURE}/${CMAKE_BUILD_TYPE}/bin )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Add subdirectory
add_subdirectory (TS_ENGINE)

#SOURCE_FILES
set (SOURCE_FILES
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/src/glad/glad.c
src/Sandbox.cpp
)

#ImGui Filter
file(GLOB ImGuiSrc
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imconfig.h
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui.h
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui_demo.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui_draw.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/backends/imgui_impl_glfw.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/backends/imgui_impl_glfw.h
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/backends/imgui_impl_opengl3.h
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/backends/imgui_impl_opengl3.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui_tables.cpp
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/imgui/imgui_widgets.cpp
)
source_group("ImGui" FILES ${ImGuiSrc})

#Layers Filter
file(GLOB LayersSrc
src/SandboxLayer.h
src/SandboxLayer.cpp
)
source_group("Layers" FILES ${LayersSrc})

#Shaders Filter
file(GLOB ShadersSrc
${CMAKE_SOURCE_DIR}/Assets/Shaders/BatchLit.vert
${CMAKE_SOURCE_DIR}/Assets/Shaders/BatchLit.frag
${CMAKE_SOURCE_DIR}/Assets/Shaders/Lit.vert
${CMAKE_SOURCE_DIR}/Assets/Shaders/Lit.frag
)
source_group("Shaders" FILES ${ShadersSrc})

# Add an additional library directory
link_directories(
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/${CMAKE_ARCHITECTURE}-windows/debug/lib
${CMAKE_SOURCE_DIR}/TS_ENGINE/Dependencies/include/assimp/build/${CMAKE_ARCHITECTURE}/lib/${CMAKE_BUILD_TYPE}
)

add_executable (TS_BatchingDemo 
${SOURCE_FILES} # SOURCE_FILES Default Filter
${LayersSrc} #Layers Filter
${ShadersSrc} #Shaders Filter
${ImGuiSrc} #ImGui Filter
)

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
    )

#Include directories
target_include_directories (TS_BatchingDemo PUBLIC 
${CMAKE_SOURCE_DIR}/src
)

#Add libraries
target_link_libraries(TS_BatchingDemo LINK_PUBLIC TS_ENGINE
opengl32.lib
glfw3.lib
assimp-vc143-mtd.lib
)
