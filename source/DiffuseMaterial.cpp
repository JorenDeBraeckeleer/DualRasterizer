#include "pch.h"

#include "DiffuseMaterial.h"

//=== Constructor ===//
DiffuseMaterial::DiffuseMaterial(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique)
    : Material(pDevice, assetFile, technique)
    , m_pMatWorldVariable{ nullptr }
    , m_pDiffuseMapVariable{ nullptr }
{
    //Transfer extra matrices from CPU to GPU
    m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
    if (!m_pMatWorldVariable->IsValid())
        std::wcout << L"m_pMatWorldVariable not valid\n";

    //Push textures to GPU
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"Variable gDiffuseMap not found\n";
}

//=== Destructor ===//
DiffuseMaterial::~DiffuseMaterial()
{
    m_pDiffuseMapVariable->Release();

    m_pMatWorldVariable->Release();
}

//=== Functions ===//
void DiffuseMaterial::UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix)
{
    //Update matrices
    const Elite::FMatrix4 worldViewProjectionMatrix{ projectionMatrix * viewMatrix * worldMatrix };
    const Elite::FMatrix4 viewInverseMatrix{ Elite::Inverse(viewMatrix) };

    m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix.data));
    m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix.data));
}

void DiffuseMaterial::UpdateResources(ID3D11ShaderResourceView* pDiffuseView, ID3D11ShaderResourceView* pNormalView, ID3D11ShaderResourceView* pSpecularView, ID3D11ShaderResourceView* pGlossinessView)
{
    //Update resources if available
    if (m_pDiffuseMapVariable->IsValid())
    {
        m_pDiffuseMapVariable->SetResource(pDiffuseView);
    }
}