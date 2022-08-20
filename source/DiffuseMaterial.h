#pragma once

#include "Material.h"

//=== DiffuseMaterial class ===//
class DiffuseMaterial final : public Material
{
public:
	//=== Constructors ===//
	DiffuseMaterial(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique);
	
	//=== Destructors ===//
	virtual ~DiffuseMaterial();
	DiffuseMaterial(const DiffuseMaterial& material) = delete;
	DiffuseMaterial(DiffuseMaterial&& material) = delete;
	DiffuseMaterial& operator=(const DiffuseMaterial& material) = delete;
	DiffuseMaterial& operator=(DiffuseMaterial&& material) = delete;

	//=== Functions ===//
	virtual void UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix) override;
	virtual void UpdateResources(ID3D11ShaderResourceView* pDiffuseView = nullptr, ID3D11ShaderResourceView* pNormalView = nullptr,
		ID3D11ShaderResourceView* pSpecularView = nullptr, ID3D11ShaderResourceView* pGlossinessView = nullptr) override;

private:
	//=== Variables ===//
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
};