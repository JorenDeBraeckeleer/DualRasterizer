#include "pch.h"

#include "TexturedMaterial.h"

//=== Constructor ===//
TexturedMaterial::TexturedMaterial(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique)
    : Material(pDevice, assetFile, technique)
    , m_pMatWorldVariable{ nullptr }
    , m_pMatViewInverseVariable{ nullptr }
    , m_pDiffuseMapVariable{ nullptr }
    , m_pNormalMapVariable{ nullptr }
    , m_pSpecularMapVariable{ nullptr }
    , m_pGlossinessMapVariable{ nullptr }
{
    //Transfer extra matrices from CPU to GPU
    m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
    if (!m_pMatWorldVariable->IsValid())
        std::wcout << L"m_pMatWorldVariable not valid\n";

    m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
    if (!m_pMatViewInverseVariable->IsValid())
        std::wcout << L"m_pMatViewInverseVariable not valid\n";

    //Push textures to GPU
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"Variable gDiffuseMap not found\n";

    m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
    if (!m_pNormalMapVariable->IsValid())
        std::wcout << L"Variable gNormalMap not found\n";

    m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
    if (!m_pSpecularMapVariable->IsValid())
        std::wcout << L"Variable gSpecularMap not found\n";

    m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
    if (!m_pGlossinessMapVariable->IsValid())
        std::wcout << L"Variable gGlossinessMap not found\n";
}

//=== Destructor ===//
TexturedMaterial::~TexturedMaterial()
{
    m_pGlossinessMapVariable->Release();
    m_pSpecularMapVariable->Release();
    m_pNormalMapVariable->Release();
    m_pDiffuseMapVariable->Release();

    m_pMatWorldVariable->Release();
    m_pMatViewInverseVariable->Release();
}

//=== Functions ===//
void TexturedMaterial::UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix)
{
    //Update matrices
    const Elite::FMatrix4 worldViewProjectionMatrix{ projectionMatrix * viewMatrix * worldMatrix };
    const Elite::FMatrix4 viewInverseMatrix{ Elite::Inverse(viewMatrix) };

    m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix.data));
    m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix.data));
    m_pMatViewInverseVariable->SetMatrix(reinterpret_cast<const float*>(&viewInverseMatrix.data));
}

void TexturedMaterial::UpdateResources(ID3D11ShaderResourceView* pDiffuseView, ID3D11ShaderResourceView* pNormalView, ID3D11ShaderResourceView* pSpecularView, ID3D11ShaderResourceView* pGlossinessView)
{
    //Update resources if available
    if (m_pDiffuseMapVariable->IsValid())
    {
        m_pDiffuseMapVariable->SetResource(pDiffuseView);
    }

    if (m_pNormalMapVariable->IsValid())
    {
        m_pNormalMapVariable->SetResource(pNormalView);
    }

    if (m_pSpecularMapVariable->IsValid())
    {
        m_pSpecularMapVariable->SetResource(pSpecularView);
    }

    if (m_pGlossinessMapVariable->IsValid())
    {
        m_pGlossinessMapVariable->SetResource(pGlossinessView);
    }
}