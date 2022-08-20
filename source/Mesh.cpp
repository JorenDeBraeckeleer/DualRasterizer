#include "pch.h"

#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Triangle.h"

//=== Constructors ===//
//- Software -//
Mesh::Mesh(const std::vector<Vertex>& vertexBuffer, const std::vector<int>& indexBuffer, PrimitiveTopology primitiveTopology)
	: m_VertexBuffer{ vertexBuffer }
	, m_IndexBuffer{ indexBuffer }
	, m_PrimitiveTopology{ primitiveTopology }
	, m_UIndexBuffer{}

	, m_AmountIndices{}
	, m_pIndexBuffer{ nullptr }
	, m_pVertexBuffer{ nullptr }
	, m_pVertexLayout{ nullptr }
	, m_pMaterial{ nullptr }
	, m_pDiffuse{ nullptr }
	, m_pNormal{ nullptr }
	, m_pSpecular{ nullptr }
	, m_pGlossiness{ nullptr }
{
	Initialize(m_IndexBuffer);
}

Mesh::Mesh(const std::vector<Vertex>& vertexBuffer, const std::vector<uint32_t>& indexBuffer, PrimitiveTopology primitiveTopology)
	: m_VertexBuffer{ vertexBuffer }
	, m_UIndexBuffer{ indexBuffer }
	, m_PrimitiveTopology{ primitiveTopology }
	, m_IndexBuffer{}

	, m_AmountIndices{}
	, m_pIndexBuffer{ nullptr }
	, m_pVertexBuffer{ nullptr }
	, m_pVertexLayout{ nullptr }
	, m_pMaterial{ nullptr }
	, m_pDiffuse{ nullptr }
	, m_pNormal{ nullptr }
	, m_pSpecular{ nullptr }
	, m_pGlossiness{ nullptr }
{
	Initialize(m_UIndexBuffer);
}

//- Hardware -//
Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Material* pMaterial, Texture* pDiffuse, Texture* pNormal, Texture* pSpecular, Texture* pGlossiness)
	: m_AmountIndices{ uint32_t(indices.size()) }
	, m_pIndexBuffer{ nullptr }
	, m_pVertexBuffer{ nullptr }
	, m_pVertexLayout{ nullptr }
	, m_pMaterial{ pMaterial }
	, m_pDiffuse{ pDiffuse }
	, m_pNormal{ pNormal }
	, m_pSpecular{ pSpecular }
	, m_pGlossiness{ pGlossiness }

	, m_VertexBuffer{}
	, m_IndexBuffer{}
	, m_UIndexBuffer{}
	, m_PrimitiveTopology{}
{
	Initialize(pDevice, vertices, indices);
}

//=== Destructor ===//
Mesh::~Mesh()
{
	//Delete all triangles from the mesh
	for (Triangle* pTriangle : m_pTriangles)
	{
		delete pTriangle;
		pTriangle = nullptr;
	}

	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}
}

//=== Functions ===//
//- Software -//
template <typename myType>
void Mesh::Initialize(const std::vector<myType>& indexBuffer)
{
	//Initialize triangles with correct primitive topology
	if (m_PrimitiveTopology == PrimitiveTopology::TriangleList)
	{
		for (size_t i{}; i < indexBuffer.size(); i += 3)
		{
			//Pull the vertex values with the indices
			const Vertex vertex0 = m_VertexBuffer[indexBuffer[i + size_t(0)]];
			const Vertex vertex1 = m_VertexBuffer[indexBuffer[i + size_t(1)]];
			const Vertex vertex2 = m_VertexBuffer[indexBuffer[i + size_t(2)]];

			//Push triangle to vector
			m_pTriangles.push_back(new Triangle{ vertex0, vertex1, vertex2 });
		}
	}
	else if (m_PrimitiveTopology == PrimitiveTopology::TriangleStrip)
	{
		for (size_t i{}; i < indexBuffer.size() - 2; i += 1)
		{
			//Check if triangle has surface (if no surface -> no triangle is made) (a triangle has no surface if 2 vertices are the same)
			if (indexBuffer[i + size_t(0)] != indexBuffer[i + size_t(1)] && indexBuffer[i + size_t(0)] != indexBuffer[i + size_t(2)] && indexBuffer[i + size_t(1)] != indexBuffer[i + size_t(2)])
			{
				//Pull the vertex values with the indices
				const Vertex vertex0 = m_VertexBuffer[indexBuffer[i + size_t(0)]];
				const Vertex vertex1 = m_VertexBuffer[indexBuffer[i + size_t(1)]];
				const Vertex vertex2 = m_VertexBuffer[indexBuffer[i + size_t(2)]];

				//Push triangle to vector, change last two vertices if it's an odd triangle (triangle-strip calculation)
				if (i % 2)
				{
					m_pTriangles.push_back(new Triangle{ vertex0, vertex2, vertex1 });
				}
				else
				{
					m_pTriangles.push_back(new Triangle{ vertex0, vertex1, vertex2 });
				}
			}
		}
	}
}

//- Hardware -//
void Mesh::Initialize(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 28;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 48;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create vertex buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData{ 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		return;
	}

	//Create the input layout
	D3DX11_PASS_DESC passDesc;
	m_pMaterial->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pVertexLayout);
	if (FAILED(result))
	{
		return;
	}

	//Create index buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		return;
	}
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Filter& filter, const Triangle::CullMode& cull, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix)
{
	//Set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Update GPU memory with matrices
	m_pMaterial->UpdateMatrices(worldMatrix, viewMatrix, projectionMatrix);

	//Update GPU memory with textures
	ID3D11ShaderResourceView* diffuseResource{ m_pDiffuse ? m_pDiffuse->GetResourceView() : nullptr };
	ID3D11ShaderResourceView* normalResource{ m_pNormal ? m_pNormal->GetResourceView() : nullptr };
	ID3D11ShaderResourceView* specularResource{ m_pSpecular ? m_pSpecular->GetResourceView() : nullptr };
	ID3D11ShaderResourceView* glossinessResource{ m_pGlossiness ? m_pGlossiness->GetResourceView() : nullptr };
	m_pMaterial->UpdateResources(diffuseResource, normalResource, specularResource, glossinessResource);

	//Render triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pMaterial->GetTechnique()->GetDesc(&techDesc);
	UINT index{};

	//Get correct technique index
	if (cull == Triangle::CullMode::Static)
	{
		//No cullmode is active -> get index from filter enum
		index = static_cast<UINT>(filter);
	}
	else
	{
		//A cullmode is active -> get index from filter and cullmode enum
		index = MakeTechniquePassIndex(filter, cull);
	}

	//Render triangle
	m_pMaterial->GetTechnique()->GetPassByIndex(index)->Apply(0, pDeviceContext);
	pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
}

UINT Mesh::MakeTechniquePassIndex(const Filter& filter, const Triangle::CullMode& cull)
{
	//Calculating index (row col based)
	UINT filterIdx{ static_cast<UINT>(filter) };
	UINT rowSize{ static_cast<UINT>(Filter::Count) };
	UINT cullIdx{ static_cast<UINT>(cull) };

	return filterIdx * rowSize + cullIdx;
}