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
    //地板
    GameObject m_Floor;
    //水
    //GameObject m_Water;
    //镜子
    GameObject m_Mirror;
    //墙体
    std::vector<GameObject> m_Walls;

    // 阴影材质
    Material m_ShadowMat;	
    // 木盒材质
    Material m_WoodBoxMat;								

    //效果声明
    BasicEffect m_BasicEffect;

    // 绘制物体的索引数组大小
    UINT m_IndexCount;
    //当前动画播放到第几帧
    int m_CurrFrame;

   


    //摄像机
    std::shared_ptr<Camera> m_Camera;
    CameraMode m_CameraMode;
 
};


#endif