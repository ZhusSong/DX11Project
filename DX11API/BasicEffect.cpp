#include "Effects.h"
#include "DX11Utility.h"
#include <RenderStates.h>
#include "EffectHelper.h"
#include "DX11Debug.h"
#include "Vertex.h"

#include <TextureManager.h>
#include "LightHelper.h"
using namespace DirectX;

# pragma warning(disable: 26812)
//BasicEffect:Impl 需先于BasicEffect的定义
//

class BasicEffect::Impl
{
public:
    // 必须显式指定
    Impl() {}
    ~Impl() = default;

public:
    template<class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    std::unique_ptr<EffectHelper> m_pEffectHelper;

    ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

    std::shared_ptr<IEffectPass> m_pCurrEffectPass;
    ComPtr<ID3D11InputLayout> m_pCurrInputLayout;
    D3D11_PRIMITIVE_TOPOLOGY m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    XMFLOAT4X4 m_World{}, m_View{}, m_Proj{};
};

//
// BasicEffect
//

namespace
{
    // BasicEffect单例
    static BasicEffect* g_pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
    if (g_pInstance)
        throw std::exception("BasicEffect is a singleton!");
    g_pInstance = this;
    pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
}

BasicEffect::BasicEffect(BasicEffect&& moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
}

BasicEffect& BasicEffect::operator=(BasicEffect&& moveFrom) noexcept
{
    pImpl.swap(moveFrom.pImpl);
    return *this;
}

BasicEffect& BasicEffect::Get()
{
    if (!g_pInstance)
        throw std::exception("BasicEffect needs an instance!");
    return *g_pInstance;
}


bool BasicEffect::InitAll(ID3D11Device* device)
{
    if (!device)
        return false;

    if (!RenderStates::IsInit())
        throw std::exception("RenderStates need to be initialized first!");

    pImpl->m_pEffectHelper = std::make_unique<EffectHelper>();

    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    // 创建顶点着色器
    pImpl->m_pEffectHelper->CreateShaderFromFile("BasicVS", L"HLSL/Basic_VS.cso", device,
        nullptr, nullptr, nullptr, blob.GetAddressOf());
    // 创建顶点布局
    HR(device->CreateInputLayout(VertexPosNormalTex::GetInputLayout(), ARRAYSIZE(VertexPosNormalTex::GetInputLayout()),
        blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

    // 创建像素着色器
    pImpl->m_pEffectHelper->CreateShaderFromFile("BasicPS", L"HLSL/Basic_PS.cso", device);


    // 创建通道
    EffectPassDesc passDesc;
    passDesc.nameVS = "BasicVS";
    passDesc.namePS = "BasicPS";
    HR(pImpl->m_pEffectHelper->AddEffectPass("Basic", device, &passDesc));

    pImpl->m_pEffectHelper->SetSamplerStateByName("g_Sam", RenderStates::SSLinearWrap.Get());

    // 设置调试对象名
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
    SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
#endif
    pImpl->m_pEffectHelper->SetDebugObjectName("BasicEffect");

    return true;
}
<<<<<<< HEAD
=======
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
>>>>>>> parent of 8da2e06 (24.2.21)

void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
    XMStoreFloat4x4(&pImpl->m_World, W);
}

void XM_CALLCONV BasicEffect::SetViewMatrix(DirectX::FXMMATRIX V)
{
    XMStoreFloat4x4(&pImpl->m_View, V);
}

void XM_CALLCONV BasicEffect::SetProjMatrix(DirectX::FXMMATRIX P)
{
<<<<<<< HEAD
    XMStoreFloat4x4(&pImpl->m_Proj, P);
=======
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
>>>>>>> parent of 8da2e06 (24.2.21)
}

void BasicEffect::SetMaterial(const Material& material)
{
    TextureManager& tm = TextureManager::Get();

    PhongMaterial phongMat{};
    phongMat.ambient = material.Get<XMFLOAT4>("$AmbientColor");
    phongMat.diffuse = material.Get<XMFLOAT4>("$DiffuseColor");
    phongMat.diffuse.w = material.Get<float>("$Opacity");
    phongMat.specular = material.Get<XMFLOAT4>("$SpecularColor");
    phongMat.specular.w = material.Has<float>("$SpecularFactor") ? material.Get<float>("$SpecularFactor") : 1.0f;
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_Material")->SetRaw(&phongMat);

    auto pStr = material.TryGet<std::string>("$Diffuse");
    pImpl->m_pEffectHelper->SetShaderResourceByName("g_DiffuseMap", pStr ? tm.GetTexture(*pStr) : tm.GetNullTexture());
}

MeshDataInput BasicEffect::GetInputData(const MeshData& meshData)
{
    MeshDataInput input;
    input.pInputLayout = pImpl->m_pCurrInputLayout.Get();
    input.topology = pImpl->m_CurrTopology;

    input.pVertexBuffers = {
        meshData.m_pVertices.Get(),
        meshData.m_pNormals.Get(),
        meshData.m_pTexcoordArrays.empty() ? nullptr : meshData.m_pTexcoordArrays[0].Get()
    };
    input.strides = { 12, 12, 8 };
    input.offsets = { 0, 0, 0 };

    input.pIndexBuffer = meshData.m_pIndices.Get();
    input.indexCount = meshData.m_IndexCount;

    return input;
}

void BasicEffect::SetDirLight(uint32_t pos, const DirectionalLight& dirLight)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_DirLight")->SetRaw(&dirLight, (sizeof dirLight) * pos, sizeof dirLight);
}

void BasicEffect::SetPointLight(uint32_t pos, const PointLight& pointLight)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_PointLight")->SetRaw(&pointLight, (sizeof pointLight) * pos, sizeof pointLight);
}

void BasicEffect::SetSpotLight(uint32_t pos, const SpotLight& spotLight)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_SpotLight")->SetRaw(&spotLight, (sizeof spotLight) * pos, sizeof spotLight);
}

void BasicEffect::SetEyePos(const DirectX::XMFLOAT3& eyePos)
{
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_EyePosW")->SetFloatVector(3, reinterpret_cast<const float*>(&eyePos));
}

void BasicEffect::SetRenderDefault()
{
    pImpl->m_pCurrEffectPass = pImpl->m_pEffectHelper->GetEffectPass("Basic");
    pImpl->m_pCurrInputLayout = pImpl->m_pVertexPosNormalTexLayout;
    pImpl->m_CurrTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

void BasicEffect::Apply(ID3D11DeviceContext* deviceContext)
{
    XMMATRIX W = XMLoadFloat4x4(&pImpl->m_World);
    XMMATRIX V = XMLoadFloat4x4(&pImpl->m_View);
    XMMATRIX P = XMLoadFloat4x4(&pImpl->m_Proj);

    XMMATRIX VP = V * P;
    XMMATRIX WInvT = XMath::InverseTranspose(W);

    W = XMMatrixTranspose(W);
    VP = XMMatrixTranspose(VP);
    WInvT = XMMatrixTranspose(WInvT);

    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_WorldInvTranspose")->SetFloatMatrix(4, 4, (FLOAT*)&WInvT);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_ViewProj")->SetFloatMatrix(4, 4, (FLOAT*)&VP);
    pImpl->m_pEffectHelper->GetConstantBufferVariable("g_World")->SetFloatMatrix(4, 4, (FLOAT*)&W);

    if (pImpl->m_pCurrEffectPass)
        pImpl->m_pCurrEffectPass->Apply(deviceContext);
}