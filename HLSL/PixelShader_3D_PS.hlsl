#include "Basic.hlsli"
//3D像素着色器
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
    //提前进行alpha裁剪，减少后续运算量
    float4 texColor = g_Tex.Sample(g_SamLinear, pIn.tex);
    //裁剪掉alpha值小于0.1的像素
    clip(texColor.a - 0.1f);
    
    //Normalized normal vector
    //标准化法向量
    pIn.normalW = normalize(pIn.normalW);
    
    // Vertex vector pointing to the eye
    //标准化指向眼睛的向量
    float3 toEyeW = normalize(g_EyePosW-pIn.posW);
    
    //Initialized to 0 
    //初始化为0
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;
    
    //默认处理
    //若存在需要绘制的反射物体，则先对光照进行反射矩阵变换
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        DirectionalLight dirLight = g_DirLight[i];
       [flatten]
        if (g_IsReflection)
        {
            dirLight.direction = mul(dirLight.direction, (float3x3) (g_Reflection));
        }
        
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    //点光源处理
    //若存在需要绘制的反射物体，则先对光照进行反射矩阵转换
    PointLight pointLight;
    [unroll]
    for (i = 0; i < 5; ++i)
    {
        pointLight = g_PointLight[i];
        [flatten]
        if (g_IsReflection)
        {
            pointLight.position = (float3) mul(float4(pointLight.position, 1.0f), g_Reflection);
        }
        ComputePointLight(g_Material, pointLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    
    //聚光灯处理
    //若存在需要绘制的反射物体，则先对光照进行反射矩阵转换
    SpotLight spotLight;
    for (i = 0; i < 5; ++i)
    {
        spotLight = g_SpotLight[i];
        [flatten]
        if (g_IsReflection)
        {
            spotLight.position = (float3) mul(float4(spotLight.position, 1.0f), g_Reflection);
            spotLight.direction = mul(spotLight.direction, (float3x3) g_Reflection);
        }
        ComputeSpotLight(g_Material, spotLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }
    //float4 texColor = g_Tex.Sample(g_SamLinear, pIn.tex);
    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.diffuse.a;
    return litColor;
}