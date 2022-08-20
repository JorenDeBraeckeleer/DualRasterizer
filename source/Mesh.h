#pragma once

#include <vector>

#include "Vertex.h"
#include "Triangle.h"

class Texture;
class Material;

//=== Mesh class ===//
class Mesh final
{
public:
	//=== Filter enum class ===//
	enum class Filter
	{
		Point = 0,
		Linear = 1,
		Anisotropic = 2,
		Count = 3,
	};

	//=== PrimitiveTopology enum class ===//
	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip,
	};

	//=== Constructors ===//
	//- Software -//
	Mesh(const std::vector<Vertex>& vertexBuffer, const std::vector<int>& indexBuffer, PrimitiveTopology primitiveTopology);
	Mesh(const std::vector<Vertex>& vertexBuffer, const std::vector<uint32_t>& indexBuffer, PrimitiveTopology primitiveTopology);
	//- Hardware -//
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Material* pMaterial,
		Texture* pDiffuse = nullptr, Texture* pNormal = nullptr, Texture* pSpecular = nullptr, Texture* pGlossiness = nullptr);

	//=== Rule of five ===//
	~Mesh();
	Mesh(const Mesh& mesh) = delete;
	Mesh(Mesh&& mesh) = delete;
	Mesh& operator=(const Mesh& mesh) = delete;
	Mesh& operator=(Mesh&& mesh) = delete;

	//=== Functions ===//
	//- Software -//
	std::vector<Triangle*> GetTriangles() { return m_pTriangles; }
private:
	template <typename myType>	//=> Templated initialize <=//
	void Initialize(const std::vector<myType>& indexBuffer);

public:
	//- Hardware -//
	void Render(ID3D11DeviceContext* pDeviceContext, const Filter& filter, const Triangle::CullMode& cull, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& viewMatrix, const Elite::FMatrix4& projectionMatrix);
private:
	void Initialize(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	UINT MakeTechniquePassIndex(const Filter& filter, const Triangle::CullMode& cull);

private:
	//=== Variables ===//
	//- Software -//
	std::vector<Vertex> m_VertexBuffer;

	const std::vector<int> m_IndexBuffer;
	const std::vector<uint32_t> m_UIndexBuffer;

	const PrimitiveTopology m_PrimitiveTopology;

	std::vector<Triangle*> m_pTriangles;

	//- Hardware -//
	uint32_t m_AmountIndices;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_pVertexLayout;

	Material* m_pMaterial;

	Texture* m_pDiffuse;
	Texture* m_pNormal;
	Texture* m_pSpecular;
	Texture* m_pGlossiness;
};