#include "GameApp.h"
#include "DX11Utility.h"
#include "DX11Debug.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
<<<<<<< HEAD
    : D3DApp(hInstance, windowName, initWidth, initHeight)
=======
	: DX11App(hInstance, windowName, initWidth, initHeight),
	m_CameraMode(CameraMode::FirstPerson),
	m_ShadowMat(),
	m_WoodBoxMat()
>>>>>>> parent of 8da2e06 (24.2.21)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();

    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{

    // 获取子类
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

    // ******************
    // 第三人称摄像机的操作
    //

    ImGuiIO& io = ImGui::GetIO();
    // 绕物体旋转
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        cam3rd->RotateX(io.MouseDelta.y * 0.01f);
        cam3rd->RotateY(io.MouseDelta.x * 0.01f);
    }
    cam3rd->Approach(-io.MouseWheel * 1.0f);

    if (ImGui::Begin("Meshes"))
    {
        ImGui::Text("Third Person Mode");
        ImGui::Text("Hold the right mouse button and drag the view");
    }
    ImGui::End();
    ImGui::Render();

    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
}

void GameApp::DrawScene()
{
<<<<<<< HEAD
    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }
=======
	assert(m_D3dImmediateContext);
	assert(m_SwapChain);

	//static float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_D3dImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_D3dImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// ******************
	// 1. 给镜面反射区域写入值1到模板缓冲区
	// 
	
	// 裁剪掉背面三角形
	// 标记镜面区域的模板值为1
	// 不写入像素颜色
	m_BasicEffect.SetWriteStencilOnly(m_D3dImmediateContext.Get(), 1);
	m_Mirror.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);

	//m_D3dImmediateContext->RSSetState(nullptr);
	//m_D3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), 1);
	//m_D3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	//m_Mirror.Draw(m_D3dImmediateContext.Get());

	// 2. 绘制不透明的反射物体
	//

	//开启反射绘制

	m_BasicEffect.SetReflectionState(true);
	m_BasicEffect.SetRenderDefaultWithStencil(m_D3dImmediateContext.Get(), 1);

	m_Walls[2].Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[3].Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_Walls[4].Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	//m_CBStates.isReflection = true;
	//D3D11_MAPPED_SUBRESOURCE mappedData;
	//HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBStates, sizeof(CBDrawingStates));
	//m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);

	// 3. 绘制不透明反射物体的阴影
	//
	m_WoodBox.SetMaterial(m_ShadowMat);
	// 反射开启，阴影开启	
	m_BasicEffect.SetShadowState(true);			
	m_BasicEffect.SetRenderNoDoubleBlend(m_D3dImmediateContext.Get(), 1);

	m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);

	// 恢复到原来的状态
	m_BasicEffect.SetShadowState(false);
	m_WoodBox.SetMaterial(m_WoodBoxMat);

	//4.绘制透明镜面
	//
	//// 关闭顺逆时针裁剪
	//// 仅对模板值为1的镜面区域绘制
	//// 透明混合
	//m_D3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	//m_D3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
	//m_D3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
	//
	//m_WoodBox.Draw(m_D3dImmediateContext.Get());
	//m_Water.Draw(m_D3dImmediateContext.Get());
	//m_Mirror.Draw(m_D3dImmediateContext.Get());
	
	//关闭反射绘制
	m_BasicEffect.SetReflectionState(false);
	m_BasicEffect.SetRenderAlphaBlendWithStencil(m_D3dImmediateContext.Get(), 1);

	m_Mirror.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	//m_CBStates.isReflection = false;
	//HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBStates, sizeof(CBDrawingStates));
	//m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);
>>>>>>> parent of 8da2e06 (24.2.21)


    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);

    m_BasicEffect.SetRenderDefault();
    m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_House.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

<<<<<<< HEAD
    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
=======
	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_WoodBox.SetMaterial(m_WoodBoxMat);

	//5.绘制透明的正常物体
	// 
	////笼子
	//m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	//水面
	//m_Water.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	
	////盒子稍微高一点
	//Transform& boxTransform = m_WoodBox.GetTransform();
	//boxTransform.SetPosition(2.0f, 0.01f, 0.0f);
	//m_WoodBox.Draw(m_D3dImmediateContext.Get());
	//boxTransform.SetPosition(-2.0f, 0.01f, 0.0f);
	//m_WoodBox.Draw(m_D3dImmediateContext.Get());

	//渲染ImGui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	HR(m_SwapChain->Present(0, 0));
>>>>>>> parent of 8da2e06 (24.2.21)
}



bool GameApp::InitResource()
{
<<<<<<< HEAD
<<<<<<< HEAD
    // ******************
    // 初始化游戏对象
    //

    // 初始化地板
    Model* pModel = m_ModelManager.CreateFromFile("asset\\ground_19.obj");
    m_Ground.SetModel(pModel);
    pModel->SetDebugObjectName("ground_19");
=======
=======
>>>>>>> parent of 8da2e06 (24.2.21)
	

	//初始化游戏对象

	ComPtr<ID3D11ShaderResourceView> texture;
	//设置材质
	Material material{};
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	m_WoodBoxMat = material;
	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	//初始化木箱
	//HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	m_WoodBox.SetBuffer(m_D3dDevice.Get(),BasicObject::CreateBox());
	//稍微抬高避免深度冲突
	m_WoodBox.GetTransform().SetPosition(0.0f, 0.01f, 5.0f);
	m_WoodBox.SetTexture(texture.Get());
	m_WoodBox.SetMaterial(material);

	//初始化地板
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_D3dDevice.Get(),
		BasicObject::CreateSprite(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetMaterial(material);
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);

	//初始化墙体
	m_Walls.resize(5);
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// 这里控制墙体五个面的生成，0和1的中间位置用于放置镜面
	//     ____     ____
	//    /| 0 |   | 1 |\
    //   /4|___|___|___|2\
    //  /_/_ _ _ _ _ _ _\_\
    // | /       3       \ |
	// |/_________________\|
	//
	//生成四面墙
	for (int i = 0; i < 5; ++i)
	{
		m_Walls[i].SetMaterial(material);
		m_Walls[i].SetTexture(texture.Get());
	}
	m_Walls[0].SetBuffer(m_D3dDevice.Get(), BasicObject::CreateSprite(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[1].SetBuffer(m_D3dDevice.Get(), BasicObject::CreateSprite(XMFLOAT2(6.0f, 8.0f), XMFLOAT2(1.5f, 2.0f)));
	m_Walls[2].SetBuffer(m_D3dDevice.Get(), BasicObject::CreateSprite(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[3].SetBuffer(m_D3dDevice.Get(), BasicObject::CreateSprite(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));
	m_Walls[4].SetBuffer(m_D3dDevice.Get(), BasicObject::CreateSprite(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 2.0f)));

	m_Walls[0].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[0].GetTransform().SetPosition(-7.0f, 3.0f, 10.0f);
	m_Walls[1].GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Walls[1].GetTransform().SetPosition(7.0f, 3.0f, 10.0f);
	m_Walls[2].GetTransform().SetRotation(-XM_PIDIV2, XM_PIDIV2, 0.0f);
	m_Walls[2].GetTransform().SetPosition(10.0f, 3.0f, 0.0f);
	m_Walls[3].GetTransform().SetRotation(-XM_PIDIV2, XM_PI, 0.0f);
	m_Walls[3].GetTransform().SetPosition(0.0f, 3.0f, -10.0f);
	m_Walls[4].GetTransform().SetRotation(-XM_PIDIV2, -XM_PIDIV2, 0.0f);
	m_Walls[4].GetTransform().SetPosition(-10.0f, 3.0f, 0.0f);

	//初始化水
	//material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	//material.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
	//HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\water.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	//m_Water.SetBuffer(m_D3dDevice.Get(),
	//	BasicObject::CreateSprite(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(10.0f, 10.0f)));
	//m_Water.SetTexture(texture.Get());
	//m_Water.SetMaterial(material);

	//初始化镜面
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\ice.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Mirror.SetBuffer(m_D3dDevice.Get(),BasicObject::CreateSprite(XMFLOAT2(8.0f, 8.0f), XMFLOAT2(1.0f, 1.0f)));
	m_Mirror.GetTransform().SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	m_Mirror.GetTransform().SetPosition(0.0f, 3.0f, 10.0f);
	m_Mirror.SetTexture(texture.Get());
	m_Mirror.SetMaterial(material);

	//初始化摄像机
	//
	
	//初始化每帧都会变动的值(摄像机)
	auto camera = std::make_shared<ThirdPersonCamera>();
	m_Camera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ViewWidth, (float)m_ViewHeight);
	camera->SetDistance(5.0f);
	camera->SetDistanceMinMax(2.0f, 14.0f);
	camera->SetRotationX(XM_PIDIV2);

	m_BasicEffect.SetViewMatrix(m_Camera->GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera->GetPosition());

	m_Camera->SetFrustum(XM_PI / 3, FormRatio(), 0.5f, 1000.0f);

	m_BasicEffect.SetProjMatrix(m_Camera->GetProjXM());
>>>>>>> parent of 8da2e06 (24.2.21)


    // 初始化房屋模型
    pModel = m_ModelManager.CreateFromFile("asset\\house.obj");
    m_House.SetModel(pModel);
    pModel->SetDebugObjectName("house");

<<<<<<< HEAD
    // 获取房屋包围盒
    XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
    BoundingBox houseBox = m_House.GetModel()->boundingbox;
    houseBox.Transform(houseBox, S);
    // 让房屋底部紧贴地面
    Transform& houseTransform = m_House.GetTransform();
    houseTransform.SetScale(0.015f, 0.015f, 0.015f);
    houseTransform.SetPosition(0.0f, -(houseBox.Center.y - houseBox.Extents.y + 1.0f), 0.0f);
=======
	// 稍微高一点位置以显示阴影
	m_BasicEffect.SetShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
	m_BasicEffect.SetRefShadowMatrix(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

	//环境光
	DirectionalLight dirLight;
	dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_BasicEffect.SetDirLight(0, dirLight);
	/*m_CBRarely.dirLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].direction = XMFLOAT3(0.0f, -1.0f, 0.0f);*/
>>>>>>> parent of 8da2e06 (24.2.21)

    // ******************
    // 初始化摄像机
    //

    auto camera = std::make_shared<ThirdPersonCamera>();
    m_pCamera = camera;

    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetTarget(XMFLOAT3(0.0f, 0.5f, 0.0f));
    camera->SetDistance(15.0f);
    camera->SetDistanceMinMax(6.0f, 100.0f);
    camera->SetRotationX(XM_PIDIV4);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);

    m_BasicEffect.SetWorldMatrix(XMMatrixIdentity());
    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_BasicEffect.SetEyePos(camera->GetPosition());

    // ******************
    // 初始化不会变化的值
    //

    // 环境光
    DirectionalLight dirLight{};
    dirLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    dirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    m_BasicEffect.SetDirLight(0, dirLight);
    // 灯光
    PointLight pointLight{};
    pointLight.position = XMFLOAT3(0.0f, 20.0f, 0.0f);
    pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    pointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    pointLight.range = 30.0f;
    m_BasicEffect.SetPointLight(0, pointLight);

    return true;
}