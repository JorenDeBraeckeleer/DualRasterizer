#include "pch.h"

#include "Material.h"

//=== Constructor ===//
Material::Material(ID3D11Device* pDevice, const std::wstring& assetFile, const char* technique)
    : m_pEffect{ nullptr }
    , m_pTechnique{ nullptr }
    , m_pMatWorldViewProjVariable{ nullptr }
{
    //Initialize effect;
    m_pEffect = LoadEffect(pDevice, assetFile);

    //Initialize technique
    m_pTechnique = m_pEffect->GetTechniqueByName(technique);
    if (!m_pTechnique->IsValid())
        std::cout << "m_pTechnique not valid" << std::endl;

    //Transfer basic matrix from CPU to GPU
    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    if (!m_pMatWorldViewProjVariable->IsValid())
        std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
}

//=== Destructor ===//
Material::~Material()
{
    m_pMatWorldViewProjVariable->Release();

    m_pTechnique->Release();
    m_pEffect->Release();
}

//=== Functions ===//
void Material::UpdateMatrices(const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix)
{
    //Update basic matrix
    const Elite::FMatrix4 worldViewProjectionMatrix{ projectionMatrix * viewMatrix * worldMatrix };
    const Elite::FMatrix4 viewInverseMatrix{ Elite::Inverse(viewMatrix) };

    m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix.data));
}

ID3DX11Effect* Material::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result = S_OK;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect;

    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
        {
            char* pErrors = (char*)pErrorBlob->GetBufferPointer();

            std::stringstream ss;
            for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                ss << pErrors[i];

            OutputDebugString(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::cout << ss.str() << std::endl;
        }
        else
        {
            std::wstringstream ss;
            ss << "EffectLoader: Failed to CreateEffectFromFile!" << std::endl << "Path: " << assetFile;
            std::wcout << ss.str() << std::endl;
            return nullptr;
        }
    }

    return pEffect;
}