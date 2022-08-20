#pragma once

#include <string>
#include <sstream>

//=== Material class ===//
class Material
{
public:
	//=== Constructor ===//
	Material(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique);
	
	//=== Rule of five ===//
	virtual ~Material();
	Material(const Material& material) = delete;
	Material(Material&& material) = delete;
	Material& operator=(const Material& material) = delete;
	Material& operator=(Material&& material) = delete;

	//=== Functions ===//
	//Getters
	ID3DX11Effect* GetDX11Effect() { return m_pEffect; }
	ID3DX11EffectTechnique* GetTechnique() { return m_pTechnique; }

	//Updates
	virtual void UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix);
	virtual void UpdateResources(ID3D11ShaderResourceView* pDiffuseView = nullptr, ID3D11ShaderResourceView* pNormalView = nullptr,
		ID3D11ShaderResourceView* pSpecularView = nullptr, ID3D11ShaderResourceView* pGlossinessView = nullptr) = 0;

protected:
	//=== Variables ===//
	ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
};