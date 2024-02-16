#pragma once
#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "DX11App.h"
#include "LightHelper.h"
#include "BasicObject.h"
#include "Camera.h"
class GameApp : public DX11App
{
public:

    //设置HLSL缓冲区
    struct CBChangeEveryDrawing
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX worldInvTranspose;
    };
    struct CBChangeEveryFrame
    {
        DirectX::XMMATRIX view;
        DirectX::XMFLOAT4 eyePos;
    };
    struct CBChangeOnResize
    {
        DirectX::XMMATRIX proj;
    };
    struct CBChangeRarely
    {
        DirectionalLight dirLight[10];
        PointLight pointLight[10];
        SpotLight spotLight[10];
        Material material;
        int numDirLight;
        int numPointLight;
        int numSpotLight;
        float pad;		// 打包保证16字节对齐
    };

    //游戏对象类，集成了一个对象所需的组件
    class GameObject
    {
    public:
        GameObject();

        //获取物体变换
        Transform& GetTransform();
        
        // 获取物体变换
        const Transform& GetTransform() const;

        //设置缓冲区
        template<class VertexType,class IndexType>
        void SetBuffer(ID3D11Device* device,const BasicObject::MeshData<VertexType, IndexType>& meshData);

        //设置纹理
        void SetTexture(ID3D11ShaderResourceView* texture);

        //绘制
        void Draw(ID3D11DeviceContext* deviceContext);

        // 设置调试对象名
        // 若缓冲区被重新设置，调试对象名也需要被重新设置
        void SetDebugObjectName(const std::string& name);
    private:
        //物体变换信息
        Transform m_Transform;
        //物体贴图
        ComPtr<ID3D11ShaderResourceView> m_Texture;
        //定点缓冲区
        ComPtr<ID3D11Buffer> m_VertexBuffer;
        //索引缓冲区
        ComPtr<ID3D11Buffer> m_IndexBuffer;
        //顶点字节大小
        UINT m_VertexStride;
        //索引树木
        UINT m_IndexCount;
       
    };
    //摄像机模式
    enum class CameraMode {
        FirstPerson,
        ThirdPerson,
        Free,
    };
  /*  struct VSConstantBuffer
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
        DirectX::XMMATRIX worldInvTranspose;

    };*/

    //struct PSConstantBuffer
    //{
    //    DirectionalLight dirLight[10];
    //    PointLight pointLight[10];
    //    SpotLight spotLight[10];
    //    Material material;
    //    int numDirLight;
    //    int numPointLight;
    //    int numSpotLight;
    //    float pad;		// 打包保证16字节对齐
    //    DirectX::XMFLOAT4 eyePos;
    //};
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
    bool ResetMesh(const BasicObject::MeshData<VertexType>& meshData);
    // float posX = 0.0f;

     //2D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout2D;
    //3D顶点输入布局
    ComPtr<ID3D11InputLayout> m_VertexLayout3D;

    //常量缓冲区
    ComPtr<ID3D11Buffer> m_ConstantBuffers[4];

    GameObject m_WoodBox;
    GameObject m_Floor;
    std::vector<GameObject> m_Walls;

    //  2D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader2D;
    //  3D顶点着色器
    ComPtr<ID3D11VertexShader> m_VertexShader3D;
    // 2D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader2D;
    // 3D像素着色器
    ComPtr<ID3D11PixelShader> m_PixelShader3D;

    CBChangeEveryFrame m_CBFrame;
    CBChangeOnResize m_CBOnResize;
    CBChangeRarely m_CBRarely;
    //// 顶点缓冲区
    //ComPtr<ID3D11Buffer> m_VertexBuffer;
    ////索引缓冲区
    //ComPtr<ID3D11Buffer> m_IndexBuffer;


    // 绘制物体的索引数组大小
    UINT m_IndexCount;
    //当前动画播放到第几帧
    int m_CurrFrame;

    //纹理
    ComPtr<ID3D11ShaderResourceView> m_Texture;
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_FireAnims; // 火焰纹理集

    //采样器状态
    ComPtr<ID3D11SamplerState> m_SamplerState;

    //摄像机
    std::shared_ptr<Camera> m_Camera;
    CameraMode m_CameraMode;

    // 用于修改用于VS的GPU常量缓冲区的变量
   // VSConstantBuffer m_VSConstantBuffer;
    // 用于修改用于PS的GPU常量缓冲区的变量
  //  PSConstantBuffer m_PSConstantBuffer;

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