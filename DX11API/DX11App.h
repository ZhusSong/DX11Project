#pragma once
#ifndef DX11App_H
#define DX11App_H

#include <wrl/client.h>
#include <string>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11_1.h>
#include "WinAPISetting.h"
#include <DirectXMath.h>
#include "DXGameTimer.h"
#include"Keyboard.h"
#include"Mouse.h"
//添加ImGui
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

class DX11App
{
public:
    //初始化
   //Initialisation
   //初期化
    DX11App(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);

    virtual ~DX11App();

    //获取应用实例的句柄
    HINSTANCE AppInstance()const;
    //获取主窗口句柄
    HWND      MainWnd()const;
    //获取屏幕宽高比
    float     FormRatio()const;

    //运行
    int Run();

    //初始化窗口及Direct3D部分
    virtual bool Init();
    //在窗口大小变动时调用
    virtual void OnResize();
    //实现每一帧更新
    virtual void UpdateScene(float dt) = 0;
    //实现每一帧绘制
    virtual void DrawScene() = 0;
    //窗口消息回调
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    //鼠标位置测试
    float m_MousePosX;
    float m_MousePosY;


protected:
    //窗口初始化
    bool InitMainWindow();
    // Direct2D初始化
    bool InitDirect2D();
    //Direct3D初始化
    bool InitDirect3D();

    bool InitImGui();
    //显示帧数
    void ShowFrameCount();

    //应用实例句柄
    HINSTANCE m_AppInstance;
    //主窗口
    HWND      m_MainWnd;
    //是否暂停
    bool      m_AppPaused;
    //是否最小化
    bool      m_Minimized;
    //是否最大化
    bool      m_Maximized;
    //窗口大小是否变化
    bool      m_Resizing;
    //是否开启4倍多重采样
    bool      m_Enable4xMsaa;
    //MSAA质量等级
    UINT      m_4xMsaaQuality;

    //计时器
    DXGameTimer m_Timer;

    //模板
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
    // Direct2D
    // D2D工厂
    ComPtr<ID2D1Factory> m_D2dFactory;
    // D2D渲染目标
    ComPtr<ID2D1RenderTarget> m_D2dRenderTarget;
    // DWrite工厂
    ComPtr<IDWriteFactory> m_DwriteFactory;
    // Direct3D 11
    //D3D11设备
    ComPtr<ID3D11Device> m_D3dDevice;
    //D3D11设备上下文
    ComPtr<ID3D11DeviceContext> m_D3dImmediateContext;
    //D3D11交换链
    ComPtr<IDXGISwapChain> m_SwapChain;
    // Direct3D 11.1
    ComPtr<ID3D11Device1> m_D3dDevice1;
    ComPtr<ID3D11DeviceContext1> m_D3dImmediateContext1;
    ComPtr<IDXGISwapChain1> m_SwapChain1;

    //深度模板缓冲区
    ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
    //渲染目标视图
    ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
    //深度模板视图
    ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
    //视口
    D3D11_VIEWPORT m_ScreenViewport;

    //输入设备
    std::unique_ptr<DirectX::Mouse> m_Mouse;
    DirectX::Mouse::ButtonStateTracker m_MouseTracker;
    std::unique_ptr<DirectX::Keyboard> m_Keyboard;
    DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker;


    //主窗口标题
    std::wstring m_MainWndName;
    //视口宽度
    int m_ViewWidth;
    //视口高度
    int m_ViewHeight;

    //每秒帧数
    int m_FrameCount = 60;
};



#endif
