#pragma once
#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "DX11App.h"
#include "LightHelper.h"
#include "BasicObject.h"
class GameApp : public DX11App
{
public:


    struct VSConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;

    };

    struct PSConstantBuffer
    {
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        Material material;
        int numDirLight;
        int numPointLight;
        int numSpotLight;
        float pad;		// 打包保证16字节对齐
        DirectX::XMFLOAT4 eyePos;
    };
    enum class ShowMode {
        Box,
        Sphere,
        Cone,
        Effect_2D,
    };
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();



    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitEffect();
    bool InitResource();

    template<class VertexType>
    bool ResetMesh(const GameObject::MeshData<VertexType>& meshData);
    // float posX = 0.0f;

     //2D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout2D;
    //3D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout3D;
    // 顶点缓冲区
    ComPtr<ID3D11Buffer> m_VertexBuffer;
    //索引缓冲区
    ComPtr<ID3D11Buffer> m_IndexBuffer;
    //常量缓冲区
    ComPtr<ID3D11Buffer> m_ConstantBuffers[2];
    // 绘制物体的索引数组大小
    UINT m_IndexCount;
    //当前动画播放到第几帧
    int m_CurrFrame;

    //纹理
    ComPtr<ID3D11ShaderResourceView> m_Texture;
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_FireAnims; // 火焰纹理集

    //采样器状态
    ComPtr<ID3D11SamplerState> m_SamplerState;

    //  2D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader2D;
    //  3D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader3D;
    // 2D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader2D;
    // 3D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader3D;

    // 用于修改用于VS的GPU常量缓冲区的变量
    VSConstantBuffer m_VSConstantBuffer;
    // 用于修改用于PS的GPU常量缓冲区的变量
    PSConstantBuffer m_PSConstantBuffer;

    //  默认环境光
    DirectionalLight m_DirLight;
    // 默认点光
    PointLight m_PointLight;
    // 默认汇聚光
    SpotLight m_SpotLight;
    // 光栅化状态: 线框模式
    ComPtr<ID3D11RasterizerState> m_RSWireframe;
    // 当前是否为线框模式
    bool m_IsWireframeMode;
    float posX = 0.0f;
    float posY = 0.0f;
    // 当前显示的模式
    ShowMode m_CurrMode;
};


#endif