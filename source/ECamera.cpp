#include "pch.h"

#include "ECamera.h"

namespace Elite
{
	Camera::Camera(bool usingSoftware, const FPoint3& position, const FVector3& viewForward, float fovAngle) :
		m_Fov(tanf((fovAngle* float(E_TO_RADIANS)) / 2.f)),
		m_Position{ position },
		m_ViewForward{ GetNormalized(-viewForward) },
		m_AspectRatio{},
		m_UsingSoftware{ usingSoftware }
	{
		//Calculate initial matrices based on given parameters (position & target)
		CalculateLookAt();
		CalculateProjection();
	}

	Camera::Camera(bool usingSoftware, float aspectRatio, const FPoint3& position, const FVector3& viewForward, float fovAngle) :
		Camera(usingSoftware, position, viewForward, fovAngle)
	{
		//Initialize with given aspectRatio
		m_AspectRatio = aspectRatio;
	}

	const FPoint3& Camera::GetPosition() const
	{
		return m_Position;
	}

	void Camera::Update(bool usingSoftware, float elapsedSec)
	{
		if (m_UsingSoftware != usingSoftware)
		{
			m_Position.z = -m_Position.z;
			m_AbsoluteRotation.y += 180;

			m_UsingSoftware = usingSoftware;
			m_ViewForward = -m_ViewForward;
		}

		//Capture Input (absolute) Rotation & (relative) Movement
		//*************
		//Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
		float keyboardSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] ? m_KeyboardMoveSensitivity * m_KeyboardMoveMultiplier : m_KeyboardMoveSensitivity;
		m_RelativeTranslation.x = (pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * keyboardSpeed * elapsedSec;
		m_RelativeTranslation.y = 0;
		if (m_UsingSoftware)
		{
			m_RelativeTranslation.z = (pKeyboardState[SDL_SCANCODE_S] - pKeyboardState[SDL_SCANCODE_W]) * keyboardSpeed * elapsedSec;
		}
		else
		{
			m_RelativeTranslation.z = (pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S]) * keyboardSpeed * elapsedSec;
		}

		//Mouse Input
		int x, y = 0;
		uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
		if (mouseState == SDL_BUTTON_LMASK)
		{
			m_RelativeTranslation.z += y * m_MouseMoveSensitivity * elapsedSec;
			m_AbsoluteRotation.y -= x * m_MouseRotationSensitivity;
		}
		else if (mouseState == SDL_BUTTON_RMASK)
		{
			m_AbsoluteRotation.x -= y * m_MouseRotationSensitivity;
			m_AbsoluteRotation.y -= x * m_MouseRotationSensitivity;
		}
		else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			m_RelativeTranslation.y -= y * m_MouseMoveSensitivity * elapsedSec;
		}

		//Update LookAt (view2world & world2view matrices)
		//*************
		CalculateLookAt();

		//Update Projection
		CalculateProjection();
	}

	void Camera::CalculateLookAt()
	{
		float coordinateSystemConstant = -m_ViewForward.z;

		//FORWARD (zAxis) with YAW applied (apply correct system rotation)
		FMatrix3 yawRotation = MakeRotationY(m_AbsoluteRotation.y * float(E_TO_RADIANS));
		FVector3 zAxis = yawRotation * m_ViewForward;

		zAxis.x = coordinateSystemConstant * zAxis.x;

		//Calculate RIGHT (xAxis) based on transformed FORWARD
		FVector3 xAxis = GetNormalized(Cross(FVector3{ 0.f,1.f,0.f }, zAxis));

		//FORWARD with PITCH applied (based on xAxis) (apply correct system rotation)
		FMatrix3 pitchRotation = MakeRotation(m_AbsoluteRotation.x * float(E_TO_RADIANS), xAxis);
		zAxis = pitchRotation * zAxis;

		zAxis.y = coordinateSystemConstant * zAxis.y;

		//Calculate UP (yAxis)
		FVector3 yAxis = Cross(zAxis, xAxis);

		//Translate based on transformed axis
		m_Position += m_RelativeTranslation.x * xAxis;
		m_Position += m_RelativeTranslation.y * yAxis;
		m_Position += m_RelativeTranslation.z * zAxis;

		//Construct View2World Matrix
		m_ViewToWorld = { FVector4(xAxis), FVector4(yAxis), FVector4(zAxis), FVector4(m_Position.x, m_Position.y, m_Position.z, 1.f) };

		//Construct World2View Matrix
		m_WorldToView = Inverse(m_ViewToWorld);
	}

	void Camera::CalculateProjection()
	{
		//Initialize far and near plane to calculate DirectX projectionMatrix
		float farPlane{ 100.0f }, nearPlane{ 0.1f };

		const Elite::FMatrix4 projectionMatrix{ 1 / (m_AspectRatio * m_Fov),	0,				0,						0,
												0,								1 / m_Fov,		0,						0,
												0,								0,				farPlane / (farPlane - nearPlane),	-(farPlane * nearPlane) / (farPlane - nearPlane),
												0,								0,				1,						0 };

		m_ProjectionMatrix = projectionMatrix;
	}
}