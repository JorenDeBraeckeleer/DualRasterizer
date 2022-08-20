/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/

#pragma once

#include "EMath.h"

namespace Elite
{
	class Camera
	{
	public:

		Camera(bool usingSoftware, const FPoint3& position = { 0.f, 0.f, 10.f }, const FVector3& viewForward = { 0.f, 0.f, -1.f }, float fovAngle = 60.f);
		Camera(bool usingSoftwarefloat, float aspectRatio, const FPoint3& position = { 0.f, 0.f, 10.f }, const FVector3& viewForward = { 0.f, 0.f, -1.f }, float fovAngle = 60.f);
		~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(bool usingSoftware, float elapsedSec);

		const FMatrix4& GetWorldToView() const { return m_WorldToView; }
		const FMatrix4& GetViewToWorld() const { return m_ViewToWorld; }
		const FPoint3& GetPosition() const;

		const float GetFov() const { return m_Fov; }

		const FMatrix4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

	private:
		void CalculateLookAt();

		void CalculateProjection();

		float m_Fov{};

		const float m_KeyboardMoveSensitivity{ 15.f };
		const float m_KeyboardMoveMultiplier{ 2.5f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };

		FPoint2 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		FPoint3 m_RelativeTranslation{};

		FPoint3 m_Position{};
		FVector3 m_ViewForward{};

		FMatrix4 m_WorldToView{};
		FMatrix4 m_ViewToWorld{};

		FMatrix4 m_ProjectionMatrix{};
		float m_AspectRatio;
		bool m_UsingSoftware;
	};
}