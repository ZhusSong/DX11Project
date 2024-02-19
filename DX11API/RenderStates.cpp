#include "RenderStates.h"
#include "DX11Utility.h"
#include "DX11Debug.h"
using namespace Microsoft:: WRL;

ComPtr<ID3D11RasterizerState>RenderStates:: RSWireframe=nullptr;
ComPtr<ID3D11RasterizerState>RenderStates::RSNoCull = nullptr;
ComPtr<ID3D11RasterizerState>RenderStates::RSCullClockWise = nullptr;

ComPtr<ID3D11SamplerState>RenderStates::SSLinearWrap = nullptr;
ComPtr<ID3D11SamplerState>RenderStates::SSAnisotropicWrap = nullptr;

ComPtr<ID3D11BlendState> RenderStates::BSAlphaToCoverage = nullptr;
ComPtr<ID3D11BlendState>RenderStates::BSNoColorWrite = nullptr;
ComPtr<ID3D11BlendState>RenderStates::BSTransparent = nullptr;
ComPtr<ID3D11BlendState>RenderStates::BSAdditive = nullptr;

ComPtr<ID3D11DepthStencilState>RenderStates::DSSWriteStencil = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSDrawWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSNoDoubleBlend = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSNoDepthTest = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSNoDepthWrite = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSNoDepthTestWithStencil = nullptr;
ComPtr<ID3D11DepthStencilState>RenderStates::DSSNoDepthWriteWhithStencil = nullptr;

bool RenderStates::IsInit()
{
	//判断是否已进行过初始化
	return RSWireframe != nullptr;
}

void RenderStates::InitAll(ID3D11Device* device)
{
	if (IsInit())
		return;
	//初始化光栅化状态
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));

	//线框模式



}