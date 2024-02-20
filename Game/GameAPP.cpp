#include "GameApp.h"
#include "DX11Utility.h"
#include "DX11Debug.h"
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: DX11App(hInstance, windowName, initWidth, initHeight),
	m_CameraMode(CameraMode::FirstPerson),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely()
	/*m_IndexCount(),
	m_CurrFrame(),
	m_CurrMode(ShowMode::Box),
	m_VSConstantBuffer(),
	m_PSConstantBuffer(),
	m_DirLight(),
	m_PointLight(),
	m_SpotLight()*/
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!DX11App::Init())
		return false;

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	DX11App::OnResize();

	//摄像机变换
	if (m_Camera != nullptr)
	{
		m_Camera->SetFrustum(XM_PI / 3, FormRatio(), 0.5f, 1000.0f);
		m_Camera->SetViewPort(0.0f, 0.0f, (float)m_ViewWidth, (float)m_ViewHeight);
		m_CBOnResize.proj = XMMatrixTranspose(m_Camera->GetProjXM());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_D3dImmediateContext->Map(m_ConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
		m_D3dImmediateContext->Unmap(m_ConstantBuffers[3].Get(), 0);

	}
}

void GameApp::UpdateScene(float dt)
{
	//获取子类
	auto camera_1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_Camera);
	auto camera_3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_Camera);

	Transform& woodBox = m_WoodBox.GetTransform();

	//输入功能测试
	//Keyboard::State state_keyboard = m_Keyboard->GetState();
	//m_KeyboardTracker.Update(state_keyboard);

	//Mouse::State state_mouse = m_Mouse->GetState();
	//m_MouseTracker.Update(state_mouse);
//	static float phi = 0.0f, theta = 0.0f;

	ImGuiIO& io = ImGui::GetIO();
	if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
	{
		//前后变量与左右变量
			// 第一人称/自由摄像机的操作
		float d1 = 0.0f, d2 = 0.0f;
		if (ImGui::IsKeyDown(ImGuiKey_W))
			d1 += dt;
		if (ImGui::IsKeyDown(ImGuiKey_S))
			d1 -= dt;
		if (ImGui::IsKeyDown(ImGuiKey_A))
			d2 -= dt;
		if (ImGui::IsKeyDown(ImGuiKey_D))
			d2 += dt;


		if (m_CameraMode == CameraMode::FirstPerson)
			camera_1st->Walk(d1 * 6.0f);
		else
			camera_1st->MoveForward(d1 * 6.0f);
		camera_1st->Move(d2 * 6.0f);

		//限制摄像机位置
		//不允许穿地
		XMFLOAT3 adjustedPos;
		XMStoreFloat3(&adjustedPos, XMVectorClamp(camera_1st->GetPositionXM(),
			XMVectorSet(-8.9f, 0.0f, -8.9f, 0.0f), XMVectorReplicate(8.9f)));
		camera_1st->SetPosition(adjustedPos);

		//仅在第一人称模式移动摄像机时移动箱子
		if (m_CameraMode == CameraMode::FirstPerson)
			woodBox.SetPosition(adjustedPos);
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			camera_1st->Pitch(io.MouseDelta.y * 0.01f);
			camera_1st->RotateY(io.MouseDelta.x * 0.01f);
		}
	}
	else if(m_CameraMode==CameraMode::ThirdPerson)
	{
		camera_3rd->SetTarget(woodBox.GetPosition());

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			camera_3rd->RotateX(io.MouseDelta.y * 0.01f);
			camera_3rd->RotateY(io.MouseDelta.x * 0.01f);
		}
		camera_3rd->Approach(-io.MouseWheel * 1.0f);
	}

	//更新观察矩阵
	XMStoreFloat4(&m_CBFrame.eyePos, m_Camera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_Camera->GetViewXM());
	if (ImGui::Begin("CameraText"))
	{
		ImGui::Text("W/S/A/D in FPS/Free camera");
		ImGui::Text("Hold the right mouse button and drag the view");
		ImGui::Text("The box moves only at First Person mode");

		static int curr_item = 0;
		static const char* modes[] = {
		"First Person",
		"Third Person",
		"Free"
		};
		if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
		{
			if (curr_item == 0 && m_CameraMode != CameraMode::FirstPerson)
			{
				if (!camera_1st)
				{
					camera_1st = std::make_shared<FirstPersonCamera>();
					camera_1st->SetFrustum(XM_PI / 3, FormRatio(), 0.5f, 1000.0f);
					m_Camera = camera_1st;
				}
				camera_1st->LookTo(woodBox.GetPosition(), 
					XMFLOAT3(0.0f, 0.0f, 1.0f),
					XMFLOAT3(0.0f, 1.0f, 0.0f));
				m_CameraMode = CameraMode::FirstPerson;
			}
			else if (curr_item == 1 && m_CameraMode != CameraMode::ThirdPerson)
			{
				if (!camera_3rd)
				{
					camera_3rd = std::make_shared<ThirdPersonCamera>();
					camera_3rd->SetFrustum(XM_PI / 3,FormRatio(), 0.5f, 1000.0f);
					m_Camera = camera_3rd;
				}
				XMFLOAT3 target = woodBox.GetPosition();
				camera_3rd->SetTarget(target);
				camera_3rd->SetDistance(8.0f);
				camera_3rd->SetDistanceMinMax(3.0f, 20.0f);

				m_CameraMode = CameraMode::ThirdPerson;
			}
			else if (curr_item == 2 && m_CameraMode != CameraMode::Free)
			{
				if (!camera_1st)
				{
					camera_1st = std::make_shared<FirstPersonCamera>();
					camera_1st->SetFrustum(XM_PI / 3, FormRatio(), 0.5f, 1000.0f);
					m_Camera = camera_1st;
				}
				//从箱子上方开始移动
				XMFLOAT3 pos = woodBox.GetPosition();
				XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
				XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
				pos.y += 3;
				camera_1st->LookTo(pos, to, up);

				m_CameraMode = CameraMode::Free;
			}
		}
		auto woodPos = woodBox.GetPosition();
		ImGui::Text("Box Position\n: %.2f %.2f %.2f", woodPos.x, woodPos.y, woodPos.z);
		auto cameraPos = m_Camera->GetPostion();
		ImGui::Text("Camera Position\n: %.2f %.2f %.2f", cameraPos.x, cameraPos.y, cameraPos.z);

		ImGui::End();
		ImGui::Render();

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_D3dImmediateContext->Map(m_ConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
		m_D3dImmediateContext->Unmap(m_ConstantBuffers[2].Get(), 0);
		//m_MousePosX = state_mouse.x;
		//m_MousePosY = state_mouse.y;
	}

	//XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	//m_VSConstantBuffer.world = XMMatrixTranspose(W);
	//m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	//if (ImGui::Begin("Texture Mapping"))
	//{
	//	static int curr_mode_item = static_cast<int>(m_CurrMode);
	//	const char* mode_strs[] = {
	//		"Box",
	//		"Sphere",
	//		"Cone",
	//		"Fire"
	//	};
	//	if (ImGui::Combo("Mode", &curr_mode_item, mode_strs, ARRAYSIZE(mode_strs)))
	//	{
	//		if (curr_mode_item == 0)
	//		{
	//			// 创建立方体
	//			m_CurrMode = ShowMode::Box;
	//			m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
	//			auto meshData = GameObject::CreateBox();
	//			ResetMesh(meshData);
	//			m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	//		}
	//		else if (curr_mode_item == 1)
	//		{
	//			// 创建球
	//			m_CurrMode = ShowMode::Sphere;
	//			m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
	//			auto meshData = GameObject::CreateSphere();
	//			ResetMesh(meshData);
	//			m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	//		}
	//		else if (curr_mode_item == 2)
	//		{
	//			// 创建圆锥
	//			m_CurrMode = ShowMode::Cone;
	//			m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
	//			auto meshData = GameObject::CreateCone();
	//			ResetMesh(meshData);
	//			m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	//		}
	//		else if (curr_mode_item == 3)
	//		{
	//			m_CurrMode = ShowMode::Effect_2D;
	//			m_CurrFrame = 0;
	//			m_D3dImmediateContext->IASetInputLayout(m_VertexLayout2D.Get());
	//			auto meshData = GameObject::CreatePlane();
	//			ResetMesh(meshData);
	//			m_D3dImmediateContext->VSSetShader(m_VertexShader2D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShader(m_PixelShader2D.Get(), nullptr, 0);
	//			m_D3dImmediateContext->PSSetShaderResources(0, 1, m_FireAnims[0].GetAddressOf());
	//		}
	//	}

	//	ImGui::Text("Material");
	//	ImGui::PushID(3);
	//	ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.material.ambient.x);
	//	ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.material.diffuse.x);
	//	ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.material.specular.x);
	//	ImGui::PopID();

	//	static int curr_light_item = 1;
	//	static const char* light_modes[] = {
	//		"Directional Light",
	//		"Point Light",
	//		"Spot Light"
	//	};

	//	ImGui::Text("Light");
	//	if (ImGui::Combo("Light Type", &curr_light_item, light_modes, ARRAYSIZE(light_modes)))
	//	{
	//		m_PSConstantBuffer.dirLight[0] = (curr_light_item == 0 ? m_DirLight : DirectionalLight());
	//		m_PSConstantBuffer.pointLight[0] = (curr_light_item == 1 ? m_PointLight : PointLight());
	//		m_PSConstantBuffer.spotLight[0] = (curr_light_item == 2 ? m_SpotLight : SpotLight());
	//	}
	//	bool light_changed = false;
	//	// 添加ID区分同名控件
	//	ImGui::PushID(curr_light_item);
	//	if (curr_light_item == 0)
	//	{
	//		ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.dirLight[0].ambient.x);
	//		ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.dirLight[0].diffuse.x);
	//		ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.dirLight[0].specular.x);
	//	}
	//	else if (curr_light_item == 1)
	//	{
	//		ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.pointLight[0].ambient.x);
	//		ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.pointLight[0].diffuse.x);
	//		ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.pointLight[0].specular.x);
	//		ImGui::InputFloat("Range", &m_PSConstantBuffer.pointLight[0].range);
	//		ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.pointLight[0].att.x);

	//	}
	//	else
	//	{
	//		ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.spotLight[0].ambient.x);
	//		ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.spotLight[0].diffuse.x);
	//		ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.spotLight[0].specular.x);
	//		ImGui::InputFloat("Spot", &m_PSConstantBuffer.spotLight[0].spot);
	//		ImGui::InputFloat("Range", &m_PSConstantBuffer.spotLight[0].range);
	//		ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.spotLight[0].att.x);
	//	}
	//	ImGui::PopID();


	//	ImGui::DragFloat("phi:", &phi, 0.01f);
	//	ImGui::DragFloat("theta:", &theta, 0.01f);
	//	if (ImGui::Checkbox("WireFrame Mode", &m_IsWireframeMode))
	//	{
	//		m_D3dImmediateContext->RSSetState(m_IsWireframeMode ? m_RSWireframe.Get() : nullptr);
	//	}
	//}
	//ImGui::End();
	//ImGui::Render();
	//if (m_CurrMode == ShowMode::Box)
	//{
	//	phi += 0.001f, theta += 0.0015f;
	//	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	//	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	//	m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	//	// 更新常量缓冲区，让立方体转起来
	//	D3D11_MAPPED_SUBRESOURCE mappedData;
	//	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	//	m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	//}
	//else if (m_CurrMode == ShowMode::Sphere)
	//{
	//	phi += 0.001f, theta += 0.0015f;
	//	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	//	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	//	m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	//	// 更新常量缓冲区，让立方体转起来
	//	D3D11_MAPPED_SUBRESOURCE mappedData;
	//	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	//	m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	//}
	//else if (m_CurrMode == ShowMode::Cone)
	//{
	//	phi += 0.0001f, theta += 0.00015f;
	//	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	//	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	//	m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	//	// 更新常量缓冲区，让立方体转起来
	//	D3D11_MAPPED_SUBRESOURCE mappedData;
	//	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	//	m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	//}
	//else if (m_CurrMode == ShowMode::Effect_2D)
	//{
	//	// 用于限制在1秒60帧
	//	static float totDeltaTime = 0;

	//	totDeltaTime += dt;
	//	if (totDeltaTime > 1.0f / 60)
	//	{
	//		totDeltaTime -= 1.0f / 60;
	//		m_CurrFrame = (m_CurrFrame + 1) % 120;
	//		m_D3dImmediateContext->PSSetShaderResources(0, 1, m_FireAnims[m_CurrFrame].GetAddressOf());
	//	}
	//}

}

void GameApp::DrawScene()
{
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
	m_D3dImmediateContext->RSSetState(nullptr);
	m_D3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), 1);
	m_D3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);


	m_Mirror.Draw(m_D3dImmediateContext.Get());

	// 2. 绘制不透明的反射物体
	//

	//开启反射绘制
	m_CBStates.isReflection = true;
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBStates, sizeof(CBDrawingStates));
	m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);

	//绘制不透明物体时，需进行顺时针裁剪
	//仅对模板值为1的镜面区域绘制
	m_D3dImmediateContext->RSSetState(RenderStates::RSCullClockWise.Get());
	m_D3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
	m_D3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	//for (auto& wall : m_Walls)
	//{
	//	wall.Draw(m_D3dImmediateContext.Get());
	//}
	m_Walls[2].Draw(m_D3dImmediateContext.Get());
	m_Walls[3].Draw(m_D3dImmediateContext.Get());
	m_Walls[4].Draw(m_D3dImmediateContext.Get());
	m_Floor.Draw(m_D3dImmediateContext.Get());

	//2.绘制透明的反射物体
	//

	 
	// 关闭顺逆时针裁剪
	// 仅对模板值为1的镜面区域绘制
	// 透明混合
	m_D3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	m_D3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSDrawWithStencil.Get(), 1);
	m_D3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
	
	m_WoodBox.Draw(m_D3dImmediateContext.Get());
	m_Water.Draw(m_D3dImmediateContext.Get());
	m_Mirror.Draw(m_D3dImmediateContext.Get());
	
	//关闭反射绘制
	m_CBStates.isReflection = false;
	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBStates, sizeof(CBDrawingStates));
	m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);


	//4.绘制不透明的正常物体
	// 
	
	m_D3dImmediateContext->RSSetState(nullptr);
	m_D3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	m_D3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	for (auto& wall : m_Walls)
		wall.Draw(m_D3dImmediateContext.Get());
	m_Floor.Draw(m_D3dImmediateContext.Get());

	//5.绘制透明的正常物体
	// 
	 
	//关闭顺逆时针裁剪
	//透明混合
	m_D3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	m_D3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	m_D3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
	
	//笼子
	m_WoodBox.Draw(m_D3dImmediateContext.Get());
	//水面
	m_Water.Draw(m_D3dImmediateContext.Get());
	////盒子稍微高一点
	//Transform& boxTransform = m_WoodBox.GetTransform();
	//boxTransform.SetPosition(2.0f, 0.01f, 0.0f);
	//m_WoodBox.Draw(m_D3dImmediateContext.Get());
	//boxTransform.SetPosition(-2.0f, 0.01f, 0.0f);
	//m_WoodBox.Draw(m_D3dImmediateContext.Get());

	//渲染ImGui
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	HR(m_SwapChain->Present(0, 0));
}

bool GameApp::InitEffect()
{
	//申请内存块
	ComPtr<ID3DBlob> blob;
	// 创建顶点着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\VertexShader_2D_VS.cso", L"HLSL\\VertexShader_2D_VS.hlsl", "VS", "vs_5_0",
		blob.ReleaseAndGetAddressOf()));
	HR(m_D3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
		m_VertexShader2D.GetAddressOf()));
	// 创建顶点布局(2D)
	HR(m_D3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_VertexLayout2D.GetAddressOf()));

	// 创建像素着色器(2D)
	HR(CreateShaderFromFile(L"HLSL\\PixelShader_2D_PS.cso", L"HLSL\\PixelShader_2D_PS.hlsl", "PS", "ps_5_0",
		blob.ReleaseAndGetAddressOf()));
	HR(m_D3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
		m_PixelShader2D.GetAddressOf()));

	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\VertexShader_3D_VS.cso", L"HLSL\\VertexShader_3D_VS.hlsl", "VS", "vs_5_0",
		blob.ReleaseAndGetAddressOf()));
	HR(m_D3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
		m_VertexShader3D.GetAddressOf()));
	// 创建顶点布局(3D)
	HR(m_D3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout,
		ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_VertexLayout3D.GetAddressOf()));

	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\PixelShader_3D_PS.cso", L"HLSL\\PixelShader_3D_PS.hlsl", "PS", "ps_5_0",
		blob.ReleaseAndGetAddressOf()));
	HR(m_D3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
		m_PixelShader3D.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	////初始化网格模型并设置到输入装配阶段
	//auto meshData = GameObject::CreateBox();
	//ResetMesh(meshData);


	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// 新建用于VS和PS的常量缓冲区
	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBDrawingStates);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[1].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[2].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[3].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesRarely);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[4].GetAddressOf()));

	//初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;
	//设置材质
	Material material{};
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
	//初始化木箱
	//HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\WoodCrate.dds", nullptr, texture.GetAddressOf()));
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\WireFence.dds", nullptr, texture.GetAddressOf()));
	m_WoodBox.SetBuffer(m_D3dDevice.Get(),BasicObject::CreateBox());
	//稍微抬高避免深度冲突
	m_WoodBox.GetTransform().SetPosition(0.0f, 0.01f, 7.5f);
	m_WoodBox.SetTexture(texture.Get());
	m_WoodBox.SetMaterial(material);

	//初始化地板
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_D3dDevice.Get(),
		BasicObject::CreateSprite(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.GetTransform().SetPosition(0.0f, -1.0f, 0.0f);
	m_Floor.SetMaterial(material);

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
	/*for (int i = 0; i < 5; ++i)
	{
		m_Walls[i].SetBuffer(m_D3dDevice.Get(),
			BasicObject::CreateSprite(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		m_Walls[i].SetMaterial(material);
		Transform& transform = m_Walls[i].GetTransform();
		transform.SetRotation(-XM_PIDIV2, XM_PIDIV2 * i, 0.0f);
		transform.SetPosition(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
		m_Walls[i].SetTexture(texture.Get());
	}*/
	//初始化水
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\water.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Water.SetBuffer(m_D3dDevice.Get(),
		BasicObject::CreateSprite(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(10.0f, 10.0f)));
	m_Water.SetTexture(texture.Get());
	m_Water.SetMaterial(material);

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
	//// 初始化火焰纹理
	//WCHAR strFile[40];
	//m_FireAnims.resize(120);
	//for (int i = 1; i <= 120; ++i)
	//{
	//	wsprintf(strFile, L"asset\\FireAnim\\Fire%03d.bmp", i);
	//	HR(CreateWICTextureFromFile(m_D3dDevice.Get(), strFile, nullptr, m_FireAnims[static_cast<size_t>(i) - 1].GetAddressOf()));
	//}
	// ******************
	// 初始化默认光照
	// 方向光
	//m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_DirLight.diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	//m_DirLight.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//m_DirLight.direction = XMFLOAT3(-0.6f, -0.6f, 0.6f);
	//// 点光
	//m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	//m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	//m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	//m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//m_PointLight.range = 25.0f;
	//// 聚光灯
	//m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	//m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	//m_SpotLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//m_SpotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//m_SpotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//m_SpotLight.spot = 12.0f;
	//m_SpotLight.range = 10000.0f;
	 
	// 初始化采样器状态
	/*D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_D3dDevice->CreateSamplerState(&sampDesc, m_SamplerState.GetAddressOf()));*/

	//**********************
	//初始化常量缓冲区的值
	//初始化每帧都会变动的值(摄像机)
	m_CameraMode = CameraMode::FirstPerson;
	auto _camera = std::make_shared<FirstPersonCamera>();
	m_Camera = _camera;
	_camera->SetViewPort(0.0f,0.0f,(float)m_ViewWidth,(float)m_ViewHeight);
	_camera->LookAt(XMFLOAT3(), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	//初始化仅在窗口大小变化时修改的值
	m_Camera->SetFrustum(XM_PI / 3, FormRatio(), 0.5f, 1000.0f);
	m_CBOnResize.proj = XMMatrixTranspose(m_Camera->GetProjXM());

	//初始化不会变动的值
	//反射
	m_CBRarely.reflection = XMMatrixTranspose(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
	//环境光
	m_CBRarely.dirLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// 灯光
	m_CBRarely.pointLight[0].position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_CBRarely.pointLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_CBRarely.pointLight[0].range = 25.0f;
	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;

	//更新一般不会被修改的常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
	m_D3dImmediateContext->Unmap(m_ConstantBuffers[3].Get(), 0);

	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[4].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesRarely), &m_CBRarely, sizeof(CBChangesRarely));
	m_D3dImmediateContext->Unmap(m_ConstantBuffers[4].Get(), 0);

	//初始化所有渲染状态
	RenderStates::InitAll(m_D3dDevice.Get());

	// 初始化光栅化状态
	/*D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(m_D3dDevice->CreateRasterizerState(&rasterizerDesc, m_RSWireframe.GetAddressOf()));*/


	// 给渲染管线各个阶段绑定好所需资源
	// 设置图元类型，设定输入布局
	m_D3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
	// 默认绑定3D着色器
	m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
	//预先绑定各自所需的缓冲区，每帧更新的缓冲区需要绑定到两个缓冲区上
	m_D3dImmediateContext->VSSetConstantBuffers(0, 1, m_ConstantBuffers[0].GetAddressOf());
	m_D3dImmediateContext->VSSetConstantBuffers(1, 1, m_ConstantBuffers[1].GetAddressOf());
	m_D3dImmediateContext->VSSetConstantBuffers(2, 1, m_ConstantBuffers[2].GetAddressOf());
	m_D3dImmediateContext->VSSetConstantBuffers(3, 1, m_ConstantBuffers[3].GetAddressOf());
	m_D3dImmediateContext->VSSetConstantBuffers(4, 1, m_ConstantBuffers[4].GetAddressOf());

	//m_D3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());

	m_D3dImmediateContext->PSSetConstantBuffers(0, 1, m_ConstantBuffers[0].GetAddressOf());
	m_D3dImmediateContext->PSSetConstantBuffers(1, 1, m_ConstantBuffers[1].GetAddressOf());
	m_D3dImmediateContext->PSSetConstantBuffers(2, 1, m_ConstantBuffers[2].GetAddressOf());
	m_D3dImmediateContext->PSSetConstantBuffers(4, 1, m_ConstantBuffers[4].GetAddressOf());

	m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
	//设置采样器描述
	m_D3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	//设置混合描述
	m_D3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_VertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_VertexLayout3D.Get(), "VertexPosNormalTexLayout");

	D3D11SetDebugObjectName(m_ConstantBuffers[0].Get(), "CBDrawing");
	D3D11SetDebugObjectName(m_ConstantBuffers[1].Get(), "CBFrame");
	D3D11SetDebugObjectName(m_ConstantBuffers[2].Get(), "CBOnResize");
	D3D11SetDebugObjectName(m_ConstantBuffers[3].Get(), "CBRarely");

	D3D11SetDebugObjectName(m_VertexShader2D.Get(), "VertexShader_2D_VS");
	D3D11SetDebugObjectName(m_VertexShader3D.Get(), "VertexShade_3D_VS");
	D3D11SetDebugObjectName(m_PixelShader2D.Get(), "PixelShader_2D_PS");
	D3D11SetDebugObjectName(m_PixelShader3D.Get(), "PixelShader_3D_PS");

	m_Floor.SetDebugObjectName("Floor");
	m_WoodBox.SetDebugObjectName("WoodBox");
	m_Walls[0].SetDebugObjectName("Walls[0]");
	m_Walls[1].SetDebugObjectName("Walls[1]");
	m_Walls[2].SetDebugObjectName("Walls[2]");
	m_Walls[3].SetDebugObjectName("Walls[3]");
	m_Walls[4].SetDebugObjectName("Walls[4]");
	m_Mirror.SetDebugObjectName("Mirror");

	return true;
}
GameApp::GameObject::GameObject() :
	m_IndexCount(),
	m_Material(),
	m_VertexStride()
{

}

Transform& GameApp::GameObject::GetTransform()
{
	return m_Transform;
}


const Transform& GameApp::GameObject::GetTransform() const
{
	return m_Transform;
}

template<class VertexType,class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device* device, const BasicObject::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &InitData, m_VertexBuffer.GetAddressOf()));

	//// 输入装配阶段的顶点缓冲区设置
	//UINT stride = sizeof(VertexType);	// 跨越字节数
	//UINT offset = 0;							// 起始偏移量

	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType) ;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &InitData, m_IndexBuffer.GetAddressOf()));
	//// 输入装配阶段的索引缓冲区设置
	//m_D3dImmediateContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_Texture = texture;
}

void GameApp::GameObject::SetMaterial(const Material& material)
{
	m_Material = material;
}

void GameApp::GameObject::Draw(ID3D11DeviceContext* deviceContext)
{
	//设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBChangesEveryDrawing cbDrawing;
	//内部转置
	XMMATRIX w = m_Transform.GetLocalToWorldMatrixXM();
	cbDrawing.world = XMMatrixTranspose(w);
	cbDrawing.worldInvTranspose = XMMatrixTranspose(InverseTranspose(w));
	cbDrawing.material = m_Material;

	//更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	//设置纹理
	deviceContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	//开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}
void GameApp::GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}