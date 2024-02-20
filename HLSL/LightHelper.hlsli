//DirectionalLight
struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};
struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 position;
    float range;
    float3 att;
    float pad;
};
//阙岝摂
struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float3 position;
    float range;
    
    float3 direction;
    float Spot;
    
    float3 att;
    float pad;
};
//Material 
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 reflect;
};
    
void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye,
out float4 ambient, out float4 diffuse, out float4 spec)
{
    //初始化输出
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    //光向量与照射方向相反
    float3 lightVec = -L.direction;
    
    //添加环境光
    ambient = mat.ambient * L.ambient;
    
    //添加漫反射光
    float diffuseFactor = dot(lightVec, normal);
    
    //使用[flatten]避免动态分支
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
out float4 ambient, out float4 diffuse, out float4 spec)
{
    //初始化
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    //从物体表面到光源的向量
    float3 lightVec = L.position - pos;
    
    //物体表面到光源的距离
    float d = length(lightVec);
    
    //灯光范围测试
    if (d > L.range)
        return;
    
    //标准化光向量
    lightVec /= d;
    
    //计算环境光
    ambient = mat.ambient * L.ambient;
    
    //漫反射与镜面计算
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);
        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
    //计算光的衰弱
    float att = 1.0f / dot(L.att, float3(1.0f, d, d * d));
    diffuse *= att;
    spec *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
out float4 ambient, out float4 diffuse, out float4 spec)
{
    //初始化
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = L.position - pos;
    
    float d = length(lightVec);
    
    if (d > L.range)
    {
        return;
    }
    
    lightVec /= d;
    
    ambient = mat.ambient * L.ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.specular.w);

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
    //计算汇聚因子与衰弱系数
    float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.Spot);
    float att = spot / dot(L.att, float3(1.0f, d, d * d));
    
    ambient *= spot;
    diffuse *= att;
    spec *= att;
}