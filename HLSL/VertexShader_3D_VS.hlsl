#include "Basic.hlsli"
//3D顶点着色器
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);
    float3 normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    
    //若存在需要绘制的反射物体，则先进行反射操作
    [flatten]
    if (g_IsReflection)
    {
        posW = mul(posW, g_Reflection);
        normalW = mul(normalW, (float3x3) g_Reflection);
    }

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.tex = vIn.tex;
    return vOut;
}