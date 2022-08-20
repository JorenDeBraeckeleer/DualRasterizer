#pragma once

#include "Material.h"

//=== TexturedMaterial class ===//
class TexturedMaterial final : public Material
{
public:
	//=== Constructors ===//
	TexturedMaterial(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique);
	
	//=== Rule of five ===//
	virtual ~TexturedMaterial();
	TexturedMaterial(const TexturedMaterial& material) = delete;
	TexturedMaterial(TexturedMaterial&& material) = delete;
	TexturedMaterial& operator=(const TexturedMaterial& material) = delete;
	TexturedMaterial& operator=(TexturedMaterial&& material) = delete;

	//=== Functions ===//
	virtual void UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix) override;
	virtual void UpdateResources(ID3D11ShaderResourceView* pDiffuseView = nullptr, ID3D11ShaderResourceView* pNormalView = nullptr,
		ID3D11ShaderResourceView* pSpecularView = nullptr, ID3D11ShaderResourceView* pGlossinessView = nullptr) override;

private:
	//=== Variables ===//
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
};