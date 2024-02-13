#include "GameApp.h"
#include "DX11Utility.h"
#include "DX11Debug.h"
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
	: DX11App(hInstance, windowName, initWidth, initHeight),
	m_IndexCount(),
	m_CurrFrame(),
	m_CurrMode(ShowMode::Box),
	m_VSConstantBuffer(),
	m_PSConstantBuffer(),
	m_DirLight(),
	m_PointLight(),
	m_SpotLight()
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
}

void GameApp::UpdateScene(float dt)
{
	static float phi = 0.0f, theta = 0.0f;

	//输入功能测试
	Keyboard::State state_keyboard = m_Keyboard->GetState();
	m_KeyboardTracker.Update(state_keyboard);
	if (m_KeyboardTracker.lastState.A)
	{
		theta += 0.01f;
	}
	if (m_KeyboardTracker.lastState.D)
	{
		theta -= 0.01f;
	}
	if (m_KeyboardTracker.lastState.W)
	{
		phi += 0.01f;
	}
	if (m_KeyboardTracker.lastState.S)
	{
		phi -= 0.01f;
	}
	//Mouse::State state_mouse = m_Mouse->GetState();
	//m_MouseTracker.Update(state_mouse);

	//m_MousePosX = state_mouse.x;
	//m_MousePosY = state_mouse.y;


	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	if (ImGui::Begin("Texture Mapping"))
	{
		static int curr_mode_item = static_cast<int>(m_CurrMode);
		const char* mode_strs[] = {
			"Box",
			"Sphere",
			"Cone",
			"Fire"
		};
		if (ImGui::Combo("Mode", &curr_mode_item, mode_strs, ARRAYSIZE(mode_strs)))
		{
			if (curr_mode_item == 0)
			{
				// 创建立方体
				m_CurrMode = ShowMode::Box;
				m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
				auto meshData = GameObject::CreateBox();
				ResetMesh(meshData);
				m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
			}
			else if (curr_mode_item == 1)
			{
				// 创建球
				m_CurrMode = ShowMode::Sphere;
				m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
				auto meshData = GameObject::CreateSphere();
				ResetMesh(meshData);
				m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
			}
			else if (curr_mode_item == 2)
			{
				// 创建圆锥
				m_CurrMode = ShowMode::Cone;
				m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
				auto meshData = GameObject::CreateCone();
				ResetMesh(meshData);
				m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
			}
			else if (curr_mode_item == 3)
			{
				m_CurrMode = ShowMode::Effect_2D;
				m_CurrFrame = 0;
				m_D3dImmediateContext->IASetInputLayout(m_VertexLayout2D.Get());
				auto meshData = GameObject::CreatePlane();
				ResetMesh(meshData);
				m_D3dImmediateContext->VSSetShader(m_VertexShader2D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShader(m_PixelShader2D.Get(), nullptr, 0);
				m_D3dImmediateContext->PSSetShaderResources(0, 1, m_FireAnims[0].GetAddressOf());
			}
		}

		ImGui::Text("Material");
		ImGui::PushID(3);
		ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.material.ambient.x);
		ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.material.diffuse.x);
		ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.material.specular.x);
		ImGui::PopID();

		static int curr_light_item = 1;
		static const char* light_modes[] = {
			"Directional Light",
			"Point Light",
			"Spot Light"
		};

		ImGui::Text("Light");
		if (ImGui::Combo("Light Type", &curr_light_item, light_modes, ARRAYSIZE(light_modes)))
		{
			m_PSConstantBuffer.dirLight[0] = (curr_light_item == 0 ? m_DirLight : DirectionalLight());
			m_PSConstantBuffer.pointLight[0] = (curr_light_item == 1 ? m_PointLight : PointLight());
			m_PSConstantBuffer.spotLight[0] = (curr_light_item == 2 ? m_SpotLight : SpotLight());
		}
		bool light_changed = false;
		// 添加ID区分同名控件
		ImGui::PushID(curr_light_item);
		if (curr_light_item == 0)
		{
			ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.dirLight[0].ambient.x);
			ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.dirLight[0].diffuse.x);
			ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.dirLight[0].specular.x);
		}
		else if (curr_light_item == 1)
		{
			ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.pointLight[0].ambient.x);
			ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.pointLight[0].diffuse.x);
			ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.pointLight[0].specular.x);
			ImGui::InputFloat("Range", &m_PSConstantBuffer.pointLight[0].range);
			ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.pointLight[0].att.x);

		}
		else
		{
			ImGui::ColorEdit3("Ambient", &m_PSConstantBuffer.spotLight[0].ambient.x);
			ImGui::ColorEdit3("Diffuse", &m_PSConstantBuffer.spotLight[0].diffuse.x);
			ImGui::ColorEdit3("Specular", &m_PSConstantBuffer.spotLight[0].specular.x);
			ImGui::InputFloat("Spot", &m_PSConstantBuffer.spotLight[0].spot);
			ImGui::InputFloat("Range", &m_PSConstantBuffer.spotLight[0].range);
			ImGui::InputFloat3("Attenutation", &m_PSConstantBuffer.spotLight[0].att.x);
		}
		ImGui::PopID();


		ImGui::DragFloat("phi:", &phi, 0.01f);
		ImGui::DragFloat("theta:", &theta, 0.01f);
		if (ImGui::Checkbox("WireFrame Mode", &m_IsWireframeMode))
		{
			m_D3dImmediateContext->RSSetState(m_IsWireframeMode ? m_RSWireframe.Get() : nullptr);
		}
	}
	ImGui::End();
	ImGui::Render();
	if (m_CurrMode == ShowMode::Box)
	{
		phi += 0.001f, theta += 0.0015f;
		XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
		m_VSConstantBuffer.world = XMMatrixTranspose(W);
		m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

		// 更新常量缓冲区，让立方体转起来
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
		m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	}
	else if (m_CurrMode == ShowMode::Sphere)
	{
		phi += 0.001f, theta += 0.0015f;
		XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
		m_VSConstantBuffer.world = XMMatrixTranspose(W);
		m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

		// 更新常量缓冲区，让立方体转起来
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
		m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	}
	else if (m_CurrMode == ShowMode::Cone)
	{
		phi += 0.0001f, theta += 0.00015f;
		XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
		m_VSConstantBuffer.world = XMMatrixTranspose(W);
		m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

		// 更新常量缓冲区，让立方体转起来
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_D3dImmediateContext->Map(m_ConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
		m_D3dImmediateContext->Unmap(m_ConstantBuffers[0].Get(), 0);
	}
	else if (m_CurrMode == ShowMode::Effect_2D)
	{
		// 用于限制在1秒60帧
		static float totDeltaTime = 0;

		totDeltaTime += dt;
		if (totDeltaTime > 1.0f / 60)
		{
			totDeltaTime -= 1.0f / 60;
			m_CurrFrame = (m_CurrFrame + 1) % 120;
			m_D3dImmediateContext->PSSetShaderResources(0, 1, m_FireAnims[m_CurrFrame].GetAddressOf());
		}
	}

}

void GameApp::DrawScene()
{
	assert(m_D3dImmediateContext);
	assert(m_SwapChain);
	//static float black[4] = { 0.0f,0.0f,0.0f,1.0f };
	m_D3dImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_D3dImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制几何模型
	m_D3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

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
	//初始化网格模型并设置到输入装配阶段
	auto meshData = GameObject::CreateBox();
	ResetMesh(meshData);


	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(m_D3dDevice->CreateBuffer(&cbd, nullptr, m_ConstantBuffers[1].GetAddressOf()));

	//初始化3D纹理
	HR(CreateDDSTextureFromFile(m_D3dDevice.Get(), L"asset\\WoodCrate.dds", nullptr,
		m_Texture.GetAddressOf()));

	// 初始化火焰纹理
	WCHAR strFile[40];
	m_FireAnims.resize(120);
	for (int i = 1; i <= 120; ++i)
	{
		wsprintf(strFile, L"asset\\FireAnim\\Fire%03d.bmp", i);
		HR(CreateWICTextureFromFile(m_D3dDevice.Get(), strFile, nullptr, m_FireAnims[static_cast<size_t>(i) - 1].GetAddressOf()));
	}
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
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_D3dDevice->CreateSamplerState(&samplerDesc, m_SamplerState.GetAddressOf()));

	//**********************
	//初始化常量缓冲区的值
	//**********************

	//初始化用于VS的常量缓冲区的值
	m_VSConstantBuffer.world = XMMatrixIdentity();
	//XMMatrixLookAtLH：（设置摄像机位置,设置焦点位置,设置正方向）
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, FormRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	//初始化用于PS的常量缓冲区的值
	//初始化材质
	m_PSConstantBuffer.pointLight[0].position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PSConstantBuffer.pointLight[0].ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	m_PSConstantBuffer.pointLight[0].diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PSConstantBuffer.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PSConstantBuffer.pointLight[0].range = 15.0f;
	m_PSConstantBuffer.numDirLight = 0;
	m_PSConstantBuffer.numPointLight = 1;
	m_PSConstantBuffer.numSpotLight = 0;

	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 5.0f);



	//	m_PSConstantBuffer.dirLight[0] = m_DirLight;
		// 注意不要忘记设置此处的观察位置，否则高亮部分会有问题
	m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

	// 更新PS常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_D3dImmediateContext->Map(m_ConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_D3dImmediateContext->Unmap(m_ConstantBuffers[1].Get(), 0);
	// ******************
	// 初始化光栅化状态
	//
	/*D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(m_D3dDevice->CreateRasterizerState(&rasterizerDesc, m_RSWireframe.GetAddressOf()));*/

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	  // 给渲染管线各个阶段绑定好所需资源
	// 设置图元类型，设定输入布局
	m_D3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_D3dImmediateContext->IASetInputLayout(m_VertexLayout3D.Get());
	// 默认绑定3D着色器
	m_D3dImmediateContext->VSSetShader(m_VertexShader3D.Get(), nullptr, 0);
	// VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
	m_D3dImmediateContext->VSSetConstantBuffers(0, 1, m_ConstantBuffers[0].GetAddressOf());
	// PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
	m_D3dImmediateContext->PSSetConstantBuffers(1, 1, m_ConstantBuffers[1].GetAddressOf());
	// 像素着色阶段设置好采样器
	m_D3dImmediateContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());
	m_D3dImmediateContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	m_D3dImmediateContext->PSSetShader(m_PixelShader3D.Get(), nullptr, 0);

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_VertexLayout2D.Get(), "VertexPosTexLayout");
	D3D11SetDebugObjectName(m_VertexLayout3D.Get(), "VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(m_ConstantBuffers[0].Get(), "VSConstantBuffer");
	D3D11SetDebugObjectName(m_ConstantBuffers[1].Get(), "PSConstantBuffer");
	D3D11SetDebugObjectName(m_VertexShader2D.Get(), "VertexShader_2D");
	D3D11SetDebugObjectName(m_PixelShader2D.Get(), "PixelShader_2D");

	return true;
}

template<class VertexType>
bool GameApp::ResetMesh(const GameObject::MeshData<VertexType>& meshData)
{
	// 释放旧资源
	m_VertexBuffer.Reset();
	m_IndexBuffer.Reset();

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(m_D3dDevice->CreateBuffer(&vbd, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf()));

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexType);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量

	m_D3dImmediateContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);



	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(m_D3dDevice->CreateBuffer(&ibd, &InitData, m_IndexBuffer.ReleaseAndGetAddressOf()));
	// 输入装配阶段的索引缓冲区设置
	m_D3dImmediateContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);



	// 设置调试对象名
	D3D11SetDebugObjectName(m_VertexBuffer.Get(), "VertexBuffer");
	D3D11SetDebugObjectName(m_IndexBuffer.Get(), "IndexBuffer");

	return true;
}