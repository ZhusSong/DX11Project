#include "Basic.hlsli"
//2D像素着色器
float4 PS(VertexPosHTex pIn):SV_Target
{
    float4 color = g_Tex.Sample(g_SamLinear, pIn.tex);
    clip(color.a - 0.1f);
    return color;

}