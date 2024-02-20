#include "GameObject.h"
#include "DX11Utility.h"
using namespace DirectX;
GameObject::GameObject() :
	m_IndexCount(),
	m_Material(),
	m_VertexStride()
{
}

Transform& GameObject::GetTransform()
{
	return m_Transform;
}


const Transform& GameObject::GetTransform() const
{
	return m_Transform;
}

void GameObject::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_Texture = texture;
}

void GameObject::SetMaterial(const Material& material)
{
	m_Material = material;
}




void GameObject::Draw(ID3D11DeviceContext* deviceContext,BasicEffect& effect)
{
	//设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	effect.SetWorldMatrix(m_Transform.GetLocalToWorldMatrixXM());
	effect.SetTexture(m_Texture.Get());
	effect.SetMaterial(m_Material);
	effect.Apply(deviceContext);

	////获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	//ComPtr<ID3D11Buffer> cBuffer = nullptr;
	//deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	//CBChangesEveryDrawing cbDrawing;
	////内部转置
	//XMMATRIX w = m_Transform.GetLocalToWorldMatrixXM();
	//cbDrawing.world = XMMatrixTranspose(w);
	//cbDrawing.worldInvTranspose = XMMatrixTranspose(InverseTranspose(w));
	//cbDrawing.material = m_Material;
	////更新常量缓冲区
	//D3D11_MAPPED_SUBRESOURCE mappedData;
	//HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	//memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	//deviceContext->Unmap(cBuffer.Get(), 0);
	////设置纹理
	//deviceContext->PSSetShaderResources(0, 1, m_Texture.GetAddressOf());
	//开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}
void GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}