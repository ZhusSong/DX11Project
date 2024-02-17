#include "LightHelper.hlsli"
Texture2D g_Tex : register(t0);
SamplerState g_SamLinear : register(s0);

//cbuffer VSConstantBuffer : register(b0)
//{
//    matrix g_World;
//    matrix g_View;
//    matrix g_Proj;
//    matrix g_WorldInvTranspose;
//}

//cbuffer PSConstantBuffer : register(b1)
//{
//    DirectionalLight g_DirLight[10];
//    PointLight g_PointLight[10];
//    SpotLight g_SpotLight[10];
//    Material g_Material;
//    int g_NumDirLight;
//    int g_NumPointLight;
//    int g_NumSpotLight;
//    float g_Pad1;

//    float3 g_EyePosW;
//    float g_Pad2;
//}

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
}

cbuffer CBChangesEveryFrame : register(b1)
{
    matrix g_View;
    float3 g_EyePosW;
}

cbuffer CBChangesOnResize : register(b2)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b3)
{
    DirectionalLight g_DirLight[10];
    PointLight g_PointLight[10];
    SpotLight g_SpotLight[10];
    Material g_Material;
    int g_NumDirLight;
    int g_NumPointLight;
    int g_NumSpotLight;
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
    float3 posW : POSITION;
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