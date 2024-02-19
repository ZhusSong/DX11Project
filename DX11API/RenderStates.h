#pragma once
//******************
//提供一些渲染状态
//******************
#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include <wrl/client.h>
#include <d3d11_1.h>
#include "WinAPISetting.h"

class RenderStates
{
public:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static bool IsInit();

	static void InitAll(ID3D11Device* device);

public:
	//光栅化状态:线框
	static ComPtr<ID3D11RasterizerState> RSWireframe;
	//光栅化状态:无背面裁剪
	static ComPtr<ID3D11RasterizerState> RSNoCull;
	//光栅化状态:顺时针裁剪
	static ComPtr<ID3D11RasterizerState> RSCullClockWise;

	//采样器状态:线性过滤
	static ComPtr<ID3D11SamplerState> SSLinearWrap;
	//采样器状态:各向异性过滤
	static ComPtr<ID3D11SamplerState> SSAnisotropicWrap;

	//混合状态:不写入颜色
	static ComPtr<ID3D11BlendState> BSNoColorWrite;
	//混合状态:透明混合
	static ComPtr<ID3D11BlendState> BSTransparent;
	//混合状态:Alpha-To-Coverage
	static ComPtr<ID3D11BlendState> BSAlphaToCoverage;
	//混合状态:加法混合
	static ComPtr<ID3D11BlendState> BSAdditive;

	//深度模板状态:写入模板值
	static ComPtr<ID3D11DepthStencilState> DSSWriteStencil;
	//深度模板状态:对指定模板值的区域进行绘制
	static ComPtr<ID3D11DepthStencilState> DSSDrawWithStencil;
	//深度模板状态:无二次混合区域
	static ComPtr<ID3D11DepthStencilState> DSSNoDoubleBlend;
	//深度模板状态:关闭深度测试
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthTest;
	//深度模板状态:仅进行深度测试，不写入深度值
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthWrite;
	//深度模板状态:关闭深度测试，对指定模板值的区域进行绘制
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthTestWithStencil;
	//深度模板状态:仅进行深度测试，不写入深度值，对指定模板值的区域进行绘制
	static ComPtr<ID3D11DepthStencilState> DSSNoDepthWriteWithStencil;
};

#endif
