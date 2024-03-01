cmake_minimum_required(VERSION 3.8)

project("newDX11Project")
set(TARGET_NAME "newDX11Project")

#设置链接器子系统为窗口
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
# 设置HLSL文件的存放位置
set(HLSL_DIR "HLSL")

# 设置资源文件的存放位置
set(ASSET_DIR "asset")
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

# 将asset文件夹中的内容添加到build过程
file(COPY ${ASSET_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# 添加所有.hlsl文件和.h文件到指定文件夹中
file(GLOB HLSL_FILES ${HLSL_DIR}/*.hlsl)

file(COPY ${HLSL_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#设置HLSL文件属性
foreach(HLSL_FILE ${HLSL_FILES})

    get_filename_component(HLSL_FDIR ${HLSL_FILE} DIRECTORY)
    get_filename_component(HLSL_FBASENAME_WE ${HLSL_FILE} NAME_WE)
    string(CONCAT HLSL_FNAME_WE ${HLSL_FDIR} / ${HLSL_FBASENAME_WE})

    string(LENGTH ${HLSL_FBASENAME_WE} LEN_FNAME_WE)
    
    math(EXPR LEN_FNAME_WE "${LEN_FNAME_WE}-2")
    string(SUBSTRING ${HLSL_FBASENAME_WE} ${LEN_FNAME_WE} 2 ENTRY_POINT)
    string(TOLOWER ${ENTRY_POINT} SHADER_TYPE)

    if ("${SHADER_TYPE}" STREQUAL "vs") 
        set(SHADER_TYPE "Vertex")
    elseif("${SHADER_TYPE}" STREQUAL "hs") 
        set(SHADER_TYPE "Hull")
    elseif("${SHADER_TYPE}" STREQUAL "ds") 
        set(SHADER_TYPE "Domain")
    elseif("${SHADER_TYPE}" STREQUAL "gs") 
        set(SHADER_TYPE "Geometry")
    elseif("${SHADER_TYPE}" STREQUAL "ps")
        set(SHADER_TYPE "Pixel")
    elseif("${SHADER_TYPE}" STREQUAL "cs")
        set(SHADER_TYPE "Compute")
    endif()
    set_source_files_properties(${HLSL_FILE} PROPERTIES
                                VS_SHADER_OBJECT_FILE_NAME HLSL/${HLSL_FBASENAME_WE}.cso
                                VS_SHADER_TYPE ${SHADER_TYPE}
                                VS_SHADER_MODEL 5.0
                                VS_SHADER_ENTRYPOINT ${ENTRY_POINT}
                                VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
                                VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>)
endforeach()



# 为项目附加包含目录，使其文件夹中的文件可被整个项目访问
include_directories(DX11API)
include_directories(DX11API/TextureLoader)
include_directories(DX11API/InputAPI)

# 设置文件目录

 set (HLSL_Files
HLSL/Basic.hlsli
HLSL/LightHelper.hlsli
HLSL/PixelShader_2D_PS.hlsl
HLSL/PixelShader_3D_PS.hlsl
HLSL/VertexShader_2D_VS.hlsl
HLSL/VertexShader_3D_VS.hlsl
 )
set (SOURCES
Game/newDX11Project.cpp
Game/GameApp.cpp
Game/GameApp.h
Game/GameObject.cpp
Game/GameObject.h
 )
set (Camera
Game/Camera.h
Game/Camera.cpp
 )
 
 set (DX11API
 #编译设置
DX11API/WinAPISetting.h
#程序时间
DX11API/DXGameTimer.cpp
DX11API/DXGameTimer.h
#DX11相关API
DX11API/DX11App.h
DX11API/DX11App.cpp
#灯光
DX11API/LightHelper.h
 )
 set (GameObjects
DX11API/Vertex.h
DX11API/Vertex.cpp
DX11API/BasicObject.h
 )
#物体移动、缩放、旋转等
 set (Transform
DX11API/Transform.h
DX11API/Transform.cpp
 )
 #渲染器描述、Effect效果描述等
 set (RenderAndEffects
DX11API/RenderStates.h
DX11API/RenderStates.cpp
DX11API/Effects.h
DX11API/EffectHelper.h
DX11API/BasicEffect.cpp
 )
 set (TextureLoader
 DX11API/TextureLoader/DDSTextureLoader.cpp
DX11API/TextureLoader/DDSTextureLoader.h
DX11API/TextureLoader/WICTextureLoader.cpp
DX11API/TextureLoader/WICTextureLoader.h
 )
 #模型读取
 set (ModelManager
 DX11API/ModelManager.cpp
DX11API/ModelManager.h
 )
 set (InputAPI
DX11API/KeyBoard.cpp
DX11API/KeyBoard.h
DX11API/Mouse.cpp
DX11API/Mouse.h
 )
 set (UtilityAPI
DX11API/DX11Debug.cpp
DX11API/DX11Debug.h
DX11API/DX11Utility.cpp
DX11API/DX11Utility.h
 )
 source_group(HLSL FILES ${HLSL_Files})
 source_group(Game FILES ${SOURCES})
 source_group(Game\\Camera FILES ${Camera})
 source_group(DX11API FILES ${DX11API})
 source_group(DX11API\\TextureLoader FILES ${TextureLoader})
 source_group(DX11API\\InputAPI FILES ${InputAPI})
 source_group(DX11API\\UtilityAPI FILES ${UtilityAPI})
 source_group(DX11API\\GameObjects FILES ${GameObjects})
 source_group(DX11API\\Transform FILES ${Transform})
 source_group(DX11API\\RenderAndEffects FILES ${RenderAndEffects})
 source_group(DX11API\\ModelManager FILES ${ModelManager})


add_executable(${TARGET_NAME} ${SOURCES}  ${Camera}
${DX11API} ${TextureLoader} ${InputAPI}  ${UtilityAPI} ${GameObjects} ${Transform} ${RenderAndEffects} ${ModelManager}
${HLSL_Files})
# 添加链接库
target_link_libraries(${TARGET_NAME}  d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib winmm.lib d2d1.lib dwrite.lib winmm.lib legacy_stdio_definitions.lib)

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