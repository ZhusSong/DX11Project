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
    std::vector<CBufferBase*> m_CBuffers;
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
    if (!pImpl->m_CBuffers.empty())
        return true;
    if(!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    ComPtr<ID3DBlob> blob;

    // 创建顶点着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\VertexShader_2D_VS.cso", L"HLSL\\VertexShader_2D_VS.hlsl", "VS", "vs_5_0",
        blob.GetAddressOf()));
    HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
        pImpl->m_VertexShader2D.GetAddressOf()));
    // 创建顶点布局(2D)
    HR(device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_VertexLayout2D.GetAddressOf()));

    // 创建像素着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\PixelShader_2D_PS.cso", L"HLSL\\PixelShader_2D_PS.hlsl", "PS", "ps_5_0",
        blob.GetAddressOf()));
    HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
        pImpl->m_PixelShader2D.GetAddressOf()));

    // 创建顶点着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\VertexShader_3D_VS.cso", L"HLSL\\VertexShader_3D_VS.hlsl", "VS", "vs_5_0",
        blob.ReleaseAndGetAddressOf()));
    HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
        pImpl->m_VertexShader3D.GetAddressOf()));
    // 创建顶点布局(3D)
    HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout,
        ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_VertexLayout3D.GetAddressOf()));

    // 创建像素着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\PixelShader_3D_PS.cso", L"HLSL\\PixelShader_3D_PS.hlsl", "PS", "ps_5_0",
        blob.ReleaseAndGetAddressOf()));
    HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
        pImpl->m_PixelShader3D.GetAddressOf()));

   pImpl->m_CBuffers.assign({
        &pImpl->m_CBDrawing,
        &pImpl->m_CBFrame,
        &pImpl->m_CBStates,
        &pImpl->m_CBOnResize,
        &pImpl->m_CBRarely});

    // 创建常量缓冲区
    for (auto& pBuffer : pImpl->m_CBuffers)
    {
        HR(pBuffer->CreateBuffer(device));
    }
    // 设置调试对象名
    D3D11SetDebugObjectName(pImpl->m_VertexLayout2D.Get(), "VertexPosTexLayout");
    D3D11SetDebugObjectName(pImpl->m_VertexLayout3D.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(pImpl->m_CBuffers[0]->cBuffer.Get(), "CBDrawing");
    D3D11SetDebugObjectName(pImpl->m_CBuffers[1]->cBuffer.Get(), "CBStates");
    D3D11SetDebugObjectName(pImpl->m_CBuffers[2]->cBuffer.Get(), "CBFrame");
    D3D11SetDebugObjectName(pImpl->m_CBuffers[3]->cBuffer.Get(), "CBOnResize");
    D3D11SetDebugObjectName(pImpl->m_CBuffers[4]->cBuffer.Get(), "CBRarely");
    D3D11SetDebugObjectName(pImpl->m_VertexShader2D.Get(), "Basic_2D_VS");
    D3D11SetDebugObjectName(pImpl->m_VertexShader3D.Get(), "Basic_3D_VS");
    D3D11SetDebugObjectName(pImpl->m_PixelShader2D.Get(), "Basic_2D_PS");
    D3D11SetDebugObjectName(pImpl->m_PixelShader3D.Get(), "Basic_3D_PS");

    return true;
}
void BasicEffect::SetRenderDefault(ID3D11DeviceContext* deviceContext)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(nullptr);
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(),nullptr,0);
    deviceContext->PSSetSamplers(0,1,RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderAlphaBlend(ID3D11DeviceContext* deviceContext)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(RenderStates::RSNoCull.Get());
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderNoDoubleBlend(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(RenderStates::RSNoCull.Get());
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), 0);
    deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetWriteStencilOnly(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(nullptr);
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), 0);
    deviceContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderDefaultWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(RenderStates::RSCullClockWise.Get());
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::SetRenderAlphaBlendWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout3D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader3D.Get(), nullptr, 0);
    deviceContext->RSSetState(RenderStates::RSNoCull.Get());
    deviceContext->PSSetShader(pImpl->m_PixelShader3D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), stencilRef);
    deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void BasicEffect::Set2DRenderDefault(ID3D11DeviceContext* deviceContext)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout2D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader2D.Get(), nullptr, 0);
    deviceContext->RSSetState(nullptr);
    deviceContext->PSSetShader(pImpl->m_PixelShader2D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void BasicEffect::Set2DRenderAlphaBlend(ID3D11DeviceContext* deviceContext)
{
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(pImpl->m_VertexLayout2D.Get());
    deviceContext->VSSetShader(pImpl->m_VertexShader2D.Get(), nullptr, 0);
    deviceContext->RSSetState(RenderStates::RSNoCull.Get());
    deviceContext->PSSetShader(pImpl->m_PixelShader2D.Get(), nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
    auto& cBuffer = pImpl->m_CBDrawing;
    cBuffer.data.world = XMMatrixTranspose(W);
    cBuffer.data.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
    auto& cBuffer = pImpl->m_CBFrame;
    cBuffer.data.view = XMMatrixTranspose(V);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
    auto& cBuffer = pImpl->m_CBOnResize;
    cBuffer.data.proj = XMMatrixTranspose(P);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetReflectionMatrix(DirectX::FXMMATRIX R)
{

    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.reflection = XMMatrixTranspose(R);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetShadowMatrix(DirectX::FXMMATRIX S)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.reflection = XMMatrixTranspose(S);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetRefShadowMatrix(DirectX::FXMMATRIX RefS)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.reflection = XMMatrixTranspose(RefS);
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetDirLight(size_t pos, const DirectionalLight& dirLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.dirLight[pos] = dirLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetPointLight(size_t pos, const PointLight& pointLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.pointLight[pos] = pointLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetSpotLight(size_t pos, const SpotLight& spotLight)
{
    auto& cBuffer = pImpl->m_CBRarely;
    cBuffer.data.spotLight[pos] = spotLight;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetMaterial(const Material& material)
{
    auto& cBuffer = pImpl->m_CBDrawing;
    cBuffer.data.material = material;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetTexture(ID3D11ShaderResourceView* texture)
{
    pImpl->m_Texture = texture;
}

void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
    auto& cBuffer = pImpl->m_CBFrame;
    cBuffer.data.eyePos = eyePos;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetReflectionState(bool isOn)
{
    auto& cBuffer = pImpl->m_CBStates;
    cBuffer.data.isReflection = isOn;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetShadowState(bool isOn)
{
    auto& cBuffer = pImpl->m_CBStates;
    cBuffer.data.isShadow = isOn;
    pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
    auto& CBuffers = pImpl->m_CBuffers;
    //将各个缓冲区绑定至渲染管线
    CBuffers[0]->BindVS(deviceContext);
    CBuffers[1]->BindVS(deviceContext);
    CBuffers[2]->BindVS(deviceContext);
    CBuffers[3]->BindVS(deviceContext);
    CBuffers[4]->BindVS(deviceContext);

    CBuffers[0]->BindPS(deviceContext);
    CBuffers[1]->BindPS(deviceContext);
    CBuffers[2]->BindPS(deviceContext);
    CBuffers[4]->BindPS(deviceContext);

    //设置纹理
    deviceContext->PSSetShaderResources(0, 1, pImpl->m_Texture.GetAddressOf());
    //若存在被修改的值，则进行常量缓冲区的更新
    if (pImpl->m_IsDirty)
    {
        pImpl->m_IsDirty = false;
        for (auto& CBuffer:CBuffers)
        {
            CBuffer->UpdateBuffer(deviceContext);
        }
    }
}
