#include "Effects.h"
#include "DX11Utility.h"
#include "EffectHelper.h"
#include "DX11Debug.h"
#include "Vertex.h"
using namespace DirectX;

//BasicEffect:Impl 需先于BasicEffect的定义
//

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

	//HLSL缓冲器描述，需按16字节进行对齐
    struct CBChangesEveryDrawing
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX worldInvTranspose;
        Material material;
    };
    struct CBDrawingStates
    {
        int isReflection;
        int isShadow;
        DirectX::XMINT2 pad;
    };
    struct CBChangesEveryFrame
    {
        DirectX::XMMATRIX view;
        DirectX::XMFLOAT3 eyePos;
        float pad;
    };
    struct CBChangesOnResize
    {
        DirectX::XMMATRIX proj;
    };
    struct CBChangesRarely
    {
        //反射矩阵
        DirectX::XMMATRIX reflection;
        DirectX::XMMATRIX shadow;
        DirectX::XMMATRIX refShadow;

        //灯光
        DirectionalLight dirLight[BasicEffect::maxLights];
        PointLight pointLight[BasicEffect::maxLights];
        SpotLight spotLight[BasicEffect::maxLights];
    };
public:
    //必须显示指定
    Impl() :m_IsDirty() {}
    ~Impl() = default;
public:
    //需要进行16字节对齐的放在前面
    
    //每次绘制时的常量缓冲区
    CBufferObject<0, CBChangesEveryDrawing> m_CBDrawing;
    //每次绘制状态变化时的常量缓冲区
    CBufferObject<1, CBDrawingStates> m_CBStates;
    //每帧绘制的常量缓冲区
    CBufferObject<2, CBChangesEveryFrame> m_CBFrame;
    //每次窗口大小变化时的常量缓冲区
    CBufferObject<3, CBChangesOnResize> m_CBOnResize;
    //不会变更的常量缓冲区
    CBufferObject<4, CBChangesRarely> m_CBRarely;
    //是否有值变更
    BOOL m_IsDirty;
    //上述缓冲区的统一管理
    std::vector<CBufferBase*> m_CBuffes;
    //2D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader2D;
    //3D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader3D;

    //2D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader2D;
    //3D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader3D;

    //2D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout2D;
    //3D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout3D;

    //纹理
    ComPtr<ID3D11ShaderResourceView> m_Texture;
};

//BasicEffect
//
namespace
{
    //单例
    static BasicEffect* g_Instance = nullptr;
}

BasicEffect::BasicEffect()
{
    if (g_Instance)
        throw std::exception("BasicEffect is a singleton!");
    g_Instance = this;
    pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{

}

BasicEffect::BasicEffect(BasicEffect&& moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

BasicEffect & BasicEffect::operator=(BasicEffect&& moveFrom)noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

BasicEffect& BasicEffect::Get()
{
    if(!g_Instance)
        throw std::exception("BasicEffect is a singleton!");
    return *g_Instance;
}

bool BasicEffect::InitAll(ID3D11Device* device)
{
    if (!device)
        return false;
    if (!pImpl->m_CBuffes.empty())
        return true;
    if(!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    ComPtr<ID3DBlob> blod;

    // 创建顶点着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\VertexShader_2D_VS.cso", L"HLSL\\VertexShader_2D_VS.hlsl", "VS", "vs_5_0",
        blod.GetAddressOf()));
    HR(device->CreateVertexShader(blod->GetBufferPointer(), blod->GetBufferSize(), nullptr,
        pImpl->m_VertexShader2D.GetAddressOf()));
    // 创建顶点布局(2D)
    HR(device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blod->GetBufferPointer(), blod->GetBufferSize(), pImpl->m_VertexLayout2D.GetAddressOf()));

    // 创建像素着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\PixelShader_2D_PS.cso", L"HLSL\\PixelShader_2D_PS.hlsl", "PS", "ps_5_0",
        blod.GetAddressOf()));
    HR(device->CreatePixelShader(blod->GetBufferPointer(), blod->GetBufferSize(), nullptr,
        pImpl->m_PixelShader2D.GetAddressOf()));

    // 创建顶点着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\VertexShader_3D_VS.cso", L"HLSL\\VertexShader_3D_VS.hlsl", "VS", "vs_5_0",
        blod.ReleaseAndGetAddressOf()));
    HR(device->CreateVertexShader(blod->GetBufferPointer(), blod->GetBufferSize(), nullptr,
        pImpl->m_VertexShader3D.GetAddressOf()));
    // 创建顶点布局(3D)
    HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout,
        ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blod->GetBufferPointer(), blod->GetBufferSize(), pImpl->m_VertexLayout3D.GetAddressOf()));

    // 创建像素着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\PixelShader_3D_PS.cso", L"HLSL\\PixelShader_3D_PS.hlsl", "PS", "ps_5_0",
        blob.ReleaseAndGetAddressOf()));
    HR(m_D3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
        m_PixelShader3D.GetAddressOf()));

}