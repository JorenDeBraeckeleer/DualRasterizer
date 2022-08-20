#pragma once

#include <vector>

#include "Vertex.h"

//=== Triangle class ===//
class Triangle final
{
public:
	//=== Constructor ===//
	Triangle(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2);

	//=== Rule of five ===//
	virtual ~Triangle() = default;
	Triangle(const Triangle& triangle) = delete;
	Triangle(Triangle&& triangle) = delete;
	Triangle& operator=(const Triangle& triangle) = delete;
	Triangle& operator=(Triangle&& triangle) = delete;

	//=== CullMode enum class ===//
	enum class CullMode
	{
		BackFaceCulling = 0,
		FrontFaceCulling = 1,
		NoCulling = 2,
		Static,
	};

	//=== Functions ===//
	void ModelToNDC(const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height);
	void ModelToWorld(const Elite::FPoint3& cameraPos, const Elite::FMatrix4& worldMatrix);
	bool FrustumCulling();
	void NDCToScreen(uint32_t width, uint32_t height);
	std::vector<Elite::FPoint2> GetBoundingBox(float width, float height);
	bool PixelInTriangle(const Elite::FPoint2& pixel, float& weight0, float& weight1, float& weight2, CullMode& cullmode);
	bool Depth(float& depthBufferPixel, float& wInterpolated, const float weight0, const float weight1, const float weight2);
	void AttributeInterpolation(Triangle* pTriangle, const float wInterpolated, const float weight0, const float weight1, const float weight2, Elite::FVector2& uvInterpolated,
		Elite::FVector3& normalInterpolated, Elite::FVector3& tangentInterpolated, Elite::FVector3& viewDirectionInterpolated, Elite::RGBColor& colorInterpolated);

private:
	//=== Variables ===//
	std::vector<Vertex> m_OriginalVertices;
	std::vector<Vertex> m_Vertices;

	float m_CurrentAngle;
};