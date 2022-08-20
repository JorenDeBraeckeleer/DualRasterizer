#pragma once

#include "EMath.h"
#include "ERGBColor.h"

//=== Vertex struct ===//
struct Vertex
{
	//=== Constructor ===//
	Vertex(const Elite::FPoint4& position = {}, const Elite::RGBColor& color = {}, const Elite::FVector2& uv = {}, const Elite::FVector3& normal = {}, const Elite::FVector3& tangent = {}, const Elite::FVector3& viewDirection = {})
		: position{ position }
		, color{ color }
		, uv{ uv }
		, normal{ normal }
		, tangent{ tangent }
		, viewDirection{ viewDirection }
	{
	}

	//=== Variables ===//
	Elite::FPoint4 position;
	Elite::RGBColor color;
	Elite::FVector2 uv;
	Elite::FVector3 normal;
	Elite::FVector3 tangent;
	Elite::FVector3 viewDirection;
};