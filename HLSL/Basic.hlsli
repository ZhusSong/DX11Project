#include "LightHelper.hlsli"
Texture2D g_Tex : register(t0);
SamplerState g_SamLinear : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}
cbuffer CBDrawingStates : register(b1)
{
    int g_IsReflection;
    int g_IsShadow;
}

cbuffer CBChangesEveryFrame : register(b2)
{
    matrix g_View;
    float3 g_EyePosW;
}

cbuffer CBChangesOnResize : register(b3)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b4)
{
    matrix g_Reflection;
    matrix g_Shadow;
    matrix m_RefShadow;
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
    //int g_NumDirLight;
    //int g_NumPointLight;
    //int g_NumSpotLight;
    //float g_Pad;
}
struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosTex
{
    float3 posL : POSITION;
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    //在世界中的位置
    float3 posW : POSITION;
    //法向量在世界中的位置
    float3 normalW : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 posH : SV_POSITION;
    float2 tex : TEXCOORD;
};
//struct VertexIn
//{
//    float3 posL : POSITION;
//    float3 normalL : NORMAL;
//    float4 color : COLOR;
//};

//struct VertexOut
//{
//    float4 posH : SV_POSITION;
//    float3 posW : POSITION;
//    float3 normalW : NORMAL; 
//    float4 color : COLOR;
//};