#pragma once
#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "DX11App.h"
#include "Camera.h"
#include "GameObject.h"
class GameApp : public DX11App
{
public:
  
    //摄像机模式
    enum class CameraMode {
        FirstPerson,
        ThirdPerson,
        Free,
    };

    public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();



    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();

 
    //盒子
    GameObject m_WoodBox;
    //盒子
    GameObject m_WoodBox_2;
    //地板
    GameObject m_Floor;
    //水
    GameObject m_Water;
    //镜子
    GameObject m_Mirror;

    float r = 0.01f;
    float cam_r = 0.1f;
    GameObject m_FireAnim;
    std::vector<ComPtr<ID3D11ShaderResourceView>> m_pFireAnims; // 火焰纹理集
    //墙体
    std::vector<GameObject> m_Walls;

    // 阴影材质
    Material m_ShadowMat;	
    // 木盒材质
    Material material{};
    Material m_WoodBoxMat;								

    DirectionalLight dirLight;
	PointLight pointLight;
    PointLight pointLight_2;
    //效果声明
    BasicEffect m_BasicEffect;

    // 绘制物体的索引数组大小
    UINT m_IndexCount;
    //当前动画播放到第几帧
    int m_CurrFrame;

    enum moveStyle
    {
        dengSu=0,
        dengJiaSu=1,
        ziYouLuoTi=2,
        xiangShang=3,
        shuiPingTouShe=4,
        xieFangTouShe=5
    }m_moveStyle;

    float speed = 2.0f;
    //加速度
    float aSpeed = 10.0f;
    float time = 3.0f;
    float g = 9.8f;
    float y;
    float Vx = 0.0f;
    float Vy = 0.0f;
    float totalTime = 0.0f;
    float Vup = 12.0f;
    float Xright = 6.0f;
    float deg = 45.0f* DirectX::XM_PI/180.0f;
    DirectX::XMFLOAT3 MoveBox(moveStyle m_moveStyle, GameObject object, float dt);
    float rot_box = 0.0f;

    //摄像机
    std::shared_ptr<Camera> m_Camera;
    CameraMode m_CameraMode;
 
};


#endif