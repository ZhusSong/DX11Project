#include "GameApp.h"
#include "DX11Utility.h"
#include "DX11Debug.h"
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: DX11App(hInstance, windowName, initWidth, initHeight),
	m_CameraMode(CameraMode::ThirdPerson),
	m_ShadowMat(),
	m_WoodBoxMat()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!DX11App::Init())
		return false;
	//初始化所有渲染状态
	RenderStates::InitAll(m_D3dDevice.Get());

	if (!m_BasicEffect.InitAll(m_D3dDevice.Get()))
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
		
		m_BasicEffect.SetProjMatrix(m_Camera->GetProjXM());

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
	m_BasicEffect.SetViewMatrix(m_Camera->GetViewXM());
	m_BasicEffect.SetEyePos(m_Camera->GetPosition());


	if (ImGui::Begin("CameraText"))
	{
		ImGui::Text("W/S/A/D in FPS/Free camera");
		ImGui::Text("Hold the right mouse button and drag the view");
		ImGui::Text("The box moves only at First Person mode");

		static int curr_item = 1;
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
				camera_3rd->SetDistance(5.0f);
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
		auto cameraPos = m_Camera->GetPosition();
		ImGui::Text("Camera Position\n: %.2f %.2f %.2f", cameraPos.x, cameraPos.y, cameraPos.z);

		ImGui::End();
		ImGui::Render();


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

	//4.绘制透明反射物体
	m_BasicEffect.SetRenderDefaultWithStencil(m_D3dImmediateContext.Get(), 1);
	m_Water.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);

	//关闭反射绘制
	m_BasicEffect.SetReflectionState(false);
	m_BasicEffect.SetRenderAlphaBlendWithStencil(m_D3dImmediateContext.Get(), 1);

	m_Mirror.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	//m_CBStates.isReflection = false;
	//HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//memcpy_s(mappedData.pData, sizeof(CBDrawingStates), &m_CBStates, sizeof(CBDrawingStates));
	//m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);


	//5.绘制不透明的正常物体
	// 
	m_BasicEffect.SetRenderDefault(m_D3dImmediateContext.Get());

	for (auto& wall : m_Walls)
		wall.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_Floor.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	
	// 6. 绘制不透明正常物体的阴影
	//
	m_WoodBox.SetMaterial(m_ShadowMat);
	m_BasicEffect.SetShadowState(true);	// 反射关闭，阴影开启
	m_BasicEffect.SetRenderNoDoubleBlend(m_D3dImmediateContext.Get(), 0);

	m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetShadowState(false);		// 阴影关闭
	m_WoodBox.SetMaterial(m_WoodBoxMat);


	//7.绘制透明的正常物体
	m_BasicEffect.SetRenderAlphaBlend(m_D3dImmediateContext.Get());
	//水面
	m_Water.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);


	//5.绘制透明的正常物体
	// 
	////笼子
	//m_WoodBox.Draw(m_D3dImmediateContext.Get(), m_BasicEffect);
	
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


bool GameApp::InitResource()
{
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
	material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	material.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\water.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Water.SetBuffer(m_D3dDevice.Get(),
		BasicObject::CreateSprite(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(10.0f, 10.0f)));
	m_Water.SetTexture(texture.Get());
	m_Water.SetMaterial(material);
	m_Water.GetTransform().SetPosition(0.0f, -0.8f, 0.0f);

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


	//初始化不会变动的值
	//反射
	m_BasicEffect.SetReflectionMatrix(XMMatrixReflect(XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));

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

	// 点光
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, -10.0f);
	pointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pointLight.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 25.0f;
	m_BasicEffect.SetPointLight(0, pointLight);
	/*m_CBRarely.pointLight[0].position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_CBRarely.pointLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_CBRarely.pointLight[0].range = 25.0f;
	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;*/

	// 设置调试对象名
	//

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
