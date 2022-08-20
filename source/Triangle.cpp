#include "pch.h"

#include "Triangle.h"

#include "Mesh.h"

//=== Constructor ===//
Triangle::Triangle(const Vertex& vertex0, const Vertex& vertex1, const Vertex& vertex2)
	: m_OriginalVertices{ vertex0, vertex1, vertex2 }
	, m_Vertices{ vertex0, vertex1, vertex2 }
	, m_CurrentAngle{}
{
}

//=== Functions ===//
void Triangle::ModelToNDC(const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height)
{
	//Calculate aspect ratio for camera view
	float aspectRatio{ float(width) / float(height) }, nearPlane{ 0.1f }, farPlane{ 100.f };

	//Matrices
	const Elite::FMatrix4 viewMatrix{ Elite::Inverse(cameraToWorld) };
	const Elite::FMatrix4 projectionMatrix{ 1 / (aspectRatio * fovAngle),	0,				0,						0,
											0,								1 / fovAngle,	0,						0,
											0,								0,				-farPlane / (farPlane - nearPlane),	-(farPlane * nearPlane) / (farPlane - nearPlane),
											0,								0,				-1,						0 };

	const Elite::FMatrix4 viewProjectionMatrix{ projectionMatrix * viewMatrix };

	for (Vertex& vertex : m_Vertices)
	{
		//Make local value to hold the position of the vertex that will be transformed
		Elite::FPoint4 pos{ vertex.position };

		//Transform all vertices from world space to camera space (view space) by multiplying them with the inverse of the camera matrix
		pos = viewProjectionMatrix * pos;

		//Transform all vertices from view space to NDC coordinates (projection space) => (perspective divide)
		pos.x /= pos.w;
		pos.y /= pos.w;
		pos.z /= pos.w;

		//Give the transformed variables of the vertex back
		vertex.position = pos;
	}
}

void Triangle::ModelToWorld(const Elite::FPoint3& cameraPos, const Elite::FMatrix4& worldMatrix)
{
	//Keep hold of original vertices
	m_Vertices = m_OriginalVertices;

	//Matrices
	const Elite::FMatrix4 worldMatrix4{ worldMatrix };
	const Elite::FMatrix3 worldMatrix3{ worldMatrix4 };

	for (Vertex& vertex : m_Vertices)
	{
		//Make local value to hold the position of the vertex that will be transformed
		Elite::FPoint4 pos{ vertex.position };
		Elite::FVector3 normal{ vertex.normal };
		Elite::FVector3 tangent{ vertex.tangent };
		Elite::FVector3 viewDir{ vertex.viewDirection };

		//Transform the normal and tangent to world space
		pos = worldMatrix4 * pos;
		normal = Elite::GetNormalized(worldMatrix3 * normal);
		tangent = Elite::GetNormalized(worldMatrix3 * tangent);
		viewDir = Elite::GetNormalized(pos.xyz - cameraPos);

		//Give the transformed variables of the vertex back
		vertex.position = pos;
		vertex.normal = normal;
		vertex.tangent = tangent;
		vertex.viewDirection = viewDir;
	}
}

bool Triangle::FrustumCulling()
{
	for (Vertex& vertex : m_Vertices)
	{
		//Check if vertex isn't in screen
		if (vertex.position.x < -1.0f || vertex.position.x > 1.0f ||
			vertex.position.y < -1.0f || vertex.position.y > 1.0f ||
			vertex.position.z < 0.0f || vertex.position.z > 1.0f)
		{
			return true;
		}
	}

	return false;
}

void Triangle::NDCToScreen(uint32_t width, uint32_t height)
{
	for (Vertex& vertex : m_Vertices)
	{
		//Make local value to hold the position of the vertex that will be transformed
		Elite::FPoint4 pos{ vertex.position };

		//Transfrom all vertices from projection space to screen space
		pos.x = ((pos.x + 1.f) / 2.f) * width;
		pos.y = ((1.f - pos.y) / 2.f) * height;

		//Give the transformed variables of the vertex back
		vertex.position = pos;
	}
}

std::vector<Elite::FPoint2> Triangle::GetBoundingBox(float width, float height)
{
	//Initialize bounding box
	std::vector<Elite::FPoint2> boudingBox;

	//Make vector for std::min_element (could also use std::min without vector)
	std::vector<float> xPos{ m_Vertices[0].position.x, m_Vertices[1].position.x, m_Vertices[2].position.x };
	std::vector<float> yPos{ m_Vertices[0].position.y, m_Vertices[1].position.y, m_Vertices[2].position.y };

	//Look for most top left point
	Elite::FPoint2 topLeft{};
	topLeft.x = std::clamp(*std::min_element(xPos.begin(), xPos.end()) - 1.f, 0.0f, width);
	topLeft.y = std::clamp(*std::min_element(yPos.begin(), yPos.end()) - 1.f, 0.0f, height);

	//Look for most bottom right point
	Elite::FPoint2 bottomRight{};
	bottomRight.x = std::clamp(*std::max_element(xPos.begin(), xPos.end()) + 1.f, 0.0f, width);
	bottomRight.y = std::clamp(*std::max_element(yPos.begin(), yPos.end()) + 1.f, 0.0f, height);

	//Push points to vector
	boudingBox.push_back(topLeft);
	boudingBox.push_back(bottomRight);

	return boudingBox;
}

bool Triangle::PixelInTriangle(const Elite::FPoint2& pixel, float& weight0, float& weight1, float& weight2, CullMode& cullmode)
{
	//Initialize area
	float dot{};
	const float totalArea{ Elite::Cross((m_Vertices[0].position - m_Vertices[2].position).xy, (m_Vertices[0].position - m_Vertices[1].position).xy) };
	Elite::FVector2 pointToSide{};

	//Calculate first edge and look if the pixel is on the good side
	const Elite::FVector2 edgeA = m_Vertices[1].position.xy - m_Vertices[0].position.xy;
	pointToSide = pixel - m_Vertices[0].position.xy;
	float signedAreaA = Elite::Cross(pointToSide, edgeA);
	weight2 = signedAreaA / totalArea;

	//Calculate second edge and look if the pixel is on the good side
	const Elite::FVector2 edgeB = m_Vertices[2].position.xy - m_Vertices[1].position.xy;
	pointToSide = pixel - m_Vertices[1].position.xy;
	float signedAreaB = Elite::Cross(pointToSide, edgeB);
	weight0 = signedAreaB / totalArea;

	//Calculate third edge and look if the pixel is on the good side
	const Elite::FVector2 edgeC = m_Vertices[0].position.xy - m_Vertices[2].position.xy;
	pointToSide = pixel - m_Vertices[2].position.xy;
	float signedAreaC = Elite::Cross(pointToSide, edgeC);
	weight1 = signedAreaC / totalArea;

	//Cullcheck -> if pixel is not inside a triangle
	switch (cullmode)
	{
	case CullMode::BackFaceCulling:

		if (signedAreaA < 0)
		{
			return false;
		}

		if (signedAreaB < 0)
		{
			return false;
		}

		if (signedAreaC < 0)
		{
			return false;
		}

		break;

	case CullMode::FrontFaceCulling:

		if (signedAreaA > 0)
		{
			return false;
		}

		if (signedAreaB > 0)
		{
			return false;
		}

		if (signedAreaC > 0)
		{
			return false;
		}

		break;

	case CullMode::NoCulling:

		Elite::FVector3 normal = Elite::GetNormalized(Elite::Cross(Elite::FVector3(m_Vertices[1].position - m_Vertices[0].position), Elite::FVector3(m_Vertices[2].position - m_Vertices[0].position)));
		Elite::FVector3 viewdirection = Elite::GetNormalized(m_Vertices[0].viewDirection * weight0 + m_Vertices[1].viewDirection * weight1 + m_Vertices[2].viewDirection * weight2);
		dot = Elite::Dot(normal, viewdirection);

		if (dot > 0)
		{
			if (signedAreaA < 0)
			{
				return false;
			}
		}
		else
		{
			if (signedAreaA > 0)
			{
				return false;
			}
		}

		if (dot > 0)
		{
			if (signedAreaB < 0)
			{
				return false;
			}
		}
		else
		{
			if (signedAreaB > 0)
			{
				return false;
			}
		}

		if (dot > 0)
		{
			if (signedAreaC < 0)
			{
				return false;
			}
		}
		else
		{
			if (signedAreaC > 0)
			{
				return false;
			}
		}

		break;

	default:
		break;
	}

	//All tests succeeded, the pixel is in the triangle
	return true;
}

bool Triangle::Depth(float& depthBufferPixel, float& wInterpolated, const float weight0, const float weight1, const float weight2)
{
	//Initialize interpolated depth
	float zInterpolated{ 1 / (((1 / m_Vertices[0].position.z) * weight0) +
								((1 / m_Vertices[1].position.z) * weight1) +
								((1 / m_Vertices[2].position.z) * weight2)) };

	wInterpolated = { 1 / (((1 / m_Vertices[0].position.w) * weight0) +
							((1 / m_Vertices[1].position.w) * weight1) +
							((1 / m_Vertices[2].position.w) * weight2)) };

	float zBuffer{ zInterpolated };

	//Overwrite z-value if the new pixel is in front of the old value
	if (zBuffer < depthBufferPixel)
	{
		depthBufferPixel = zBuffer;
		return true;
	}

	return false;
}

void Triangle::AttributeInterpolation(Triangle* pTriangle, const float wInterpolated, const float weight0, const float weight1, const float weight2, Elite::FVector2& uvInterpolated,
	Elite::FVector3& normalInterpolated, Elite::FVector3& tangentInterpolated, Elite::FVector3& viewDirectionInterpolated, Elite::RGBColor& colorInterpolated)
{
	//Calculate interpolated attributes
	uvInterpolated = (((m_Vertices[0].uv / m_Vertices[0].position.w) * weight0) +
		((m_Vertices[1].uv / m_Vertices[1].position.w) * weight1) +
		((m_Vertices[2].uv / m_Vertices[2].position.w) * weight2)) * wInterpolated;


	normalInterpolated = (((m_Vertices[0].normal / m_Vertices[0].position.w) * weight0) +
		((m_Vertices[1].normal / m_Vertices[1].position.w) * weight1) +
		((m_Vertices[2].normal / m_Vertices[2].position.w) * weight2)) * wInterpolated;
	Elite::Normalize(normalInterpolated);

	tangentInterpolated = (((m_Vertices[0].tangent / m_Vertices[0].position.w) * weight0) +
		((m_Vertices[1].tangent / m_Vertices[1].position.w) * weight1) +
		((m_Vertices[2].tangent / m_Vertices[2].position.w) * weight2)) * wInterpolated;
	Elite::Normalize(tangentInterpolated);

	viewDirectionInterpolated = (((m_Vertices[0].viewDirection / m_Vertices[0].position.w) * weight0) +
		((m_Vertices[1].viewDirection / m_Vertices[1].position.w) * weight1) +
		((m_Vertices[2].viewDirection / m_Vertices[2].position.w) * weight2)) * wInterpolated;
	Elite::Normalize(viewDirectionInterpolated);

	colorInterpolated = m_Vertices[0].color * weight0 + m_Vertices[1].color * weight1 + m_Vertices[2].color * weight2;
}