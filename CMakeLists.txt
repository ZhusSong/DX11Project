cmake_minimum_required(VERSION 3.18)

project("newDX11Project")
set(TARGET_NAME "newDX11Project")

# 设置HLSL文件的存放位置
set(HLSL_DIR "HLSL")

# Win7系统设置为ON
set(WIN7_SYSTEM OFF)

# 使用ImGui时设置为ON
# 需要将项目中的ImGui文件夹复制进来，保留*.h/*.cpp/CMakeLists.txt
set(USE_IMGUI ON)

# 使用Assimp时设置为ON
# 需要将解压的assimp文件夹内CMakeList.txt所在的全部文件复制到当前项目的Assimp文件夹内
set(USE_ASSIMP ON)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)
# 设定项目编译为UNICODE标准
add_compile_definitions(UNICODE _UNICODE)

# 设置WIN7兼容
if (${WIN7_SYSTEM})
    add_compile_definitions(_WIN32_WINNT=0x601)
endif()

# 设置MSVC编译格式为utf-8
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# 将HLSL文件夹中的内容添加到build过程
file(COPY ${HLSL_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# 添加所有.hlsl文件和.h文件到指定文件夹中
file(GLOB HLSL_FILES ${HLSL_DIR}/*.hlsl)

# 设置文件目录
set (SOURCES
GameApp/main.cpp
 )
 
 set (DX11API
DX11API/WinAPISetting.h
DX11API/DXGameTimer.cpp
DX11API/DXGameTimer.h
 )
 
 set (TextureLoader
 DX11API/TextureLoader/DDSTextureLoader.cpp
DX11API/TextureLoader/DDSTextureLoader.h
DX11API/TextureLoader/WICTextureLoader.cpp
DX11API/TextureLoader/WICTextureLoader.h
 )
 
 source_group(GameApp FILES ${SOURCES})
 source_group(DX11API FILES ${DX11API})
 source_group(DX11API\\TextureLoader FILES ${TextureLoader})


add_executable(newDX11Project ${SOURCES} ${DX11API} ${TextureLoader})
# 添加链接库
target_link_libraries(${TARGET_NAME} d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib winmm.lib d2d1.lib dwrite.lib)

# 为项目调试设置输出目录
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})

if (${USE_IMGUI})
    add_subdirectory(ImGui)
    target_compile_definitions(${TARGET_NAME} PRIVATE USE_IMGUI)
    target_link_libraries(${TARGET_NAME} ImGui)
    file(COPY ImGui/imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (${USE_ASSIMP})
    set(ASSIMP_BUILD_ZLIB ON)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ASSIMP_INSTALL OFF)
    set(BUILD_SHARED_LIBS OFF)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
    add_subdirectory("assimp")
    target_link_libraries(${TARGET_NAME} assimp)
endif()