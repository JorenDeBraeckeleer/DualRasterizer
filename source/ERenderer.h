/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>

//-------------------------//
#include <memory>
#include <vector>

#include "ECamera.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Triangle.h"

class Material;
class DiffuseMaterial;
class TexturedMaterial;
class Texture;
//-------------------------//

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();

		//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
		//=== Initialze ===//
		void InitVehicle();

		//=== Software pipeline ===//
		void RenderSoftware();
		void ProjectionStage(Triangle* pTriangle, const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height, const Elite::FPoint3& cameraPos);
		void ModelToWorld(Triangle* pTriangle, const Elite::FPoint3& cameraPos, float elapsedTime);
		void ModelToNDC(Triangle* pTriangle, const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height);
		void RasterizationStage(Triangle* pTriangle, uint32_t width, uint32_t height, float* depthBuffer);
		bool FrustumCulling(Triangle* pTriangle);
		void NDCToScreen(Triangle* pTriangle, uint32_t width, uint32_t height);
		bool PixelInTriangle(Triangle* pTriangle, const Elite::FPoint2& pixel, float& weight0, float& weight1, float& weight2);
		bool Depth(Triangle* pTriangle, float& depthBufferPixel, float& wInterpolated, const float weight0, const float weight1, const float weight2);
		void AttributeInterpolation(Triangle* pTriangle, const float wInterpolated, const float weight0, const float weight1, const float weight2, Elite::FVector2& uvInterpolated,
			Elite::FVector3& normalInterpolated, Elite::FVector3& tangentInterpolated, Elite::FVector3& viewDirectionInterpolated, Elite::RGBColor& colorInterpolated);
		Elite::RGBColor PixelShadingStage(const Elite::FVector2& uvInterpolated, const Elite::FVector3& normalInterpolated, const Elite::FVector3& tangentInterpolated,
			const Elite::FVector3& viewDirectionInterpolated, const Elite::RGBColor& colorInterpolated);
		Elite::FVector3 NormalMapping(const Elite::FVector2& uvInterpolated, const Elite::FVector3& normalInterpolated, const Elite::FVector3& tangentInterpolated);
		Elite::RGBColor Diffuse(const Elite::FVector2& uvInterpolated, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection, const Elite::RGBColor& lightColor,
			const float lightIntensity);
		Elite::RGBColor Specular(const Elite::FVector3& viewDirectionInterpolated, const Elite::FVector2& uvInterpolated, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection,
			const float shininess, float& specularReflectance);
		Elite::RGBColor Ambient();

		//=== Hardware pipeline ===//
		void RenderHardware();

		//=== Update ===//
		void Update(float elapsedSec);
		void SetElapsedTime(float elapsedTime) { m_ElapsedTime = elapsedTime; }

		//=== Keybindings ===//
		void InfoKeys(SDL_Scancode key);
		void InfoText();
		//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

	private:
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;

		bool m_IsInitialized;

		//----------------------------------//
		//=== Variables ===//
		bool m_UsingSoftware = false;

		std::unique_ptr<Camera> m_pCamera;

		Texture* m_pTexture = nullptr;
		Texture* m_pNormal = nullptr;
		Texture* m_pSpecular = nullptr;
		Texture* m_pGloss = nullptr;

		float m_ElapsedTime = 0.f;

		bool m_IsRotating = true;

		Triangle::CullMode m_CullMode = Triangle::CullMode::BackFaceCulling;

		//- Software -//
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;

		float* m_pDepthBufferPixels = nullptr;

		std::vector<Triangle*> m_pTriangles;
		std::vector<Mesh*> m_pSoftwareMeshes;

		bool m_IsNormalMapping = true;
		bool m_IsDepthBufferColor = false;

		//- Hardware -//

		TexturedMaterial* m_pVehicleEffect = nullptr;
		DiffuseMaterial* m_pFireEffect = nullptr;

		Texture* m_pFireDiffuse = nullptr;

		Mesh::Filter m_Filter = Mesh::Filter::Point;

		std::vector<Mesh*> m_pHardwareMeshes;
		Mesh* m_pFireMesh;

		bool m_ShowFireMesh = true;

		float m_RotationAngleL = 0.0f;
		float m_RotationAngleR = 0.0f;
		Elite::FMatrix4 m_WorldMatrix = Elite::FMatrix4::Identity();

		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pDeviceContext = nullptr;
		IDXGIFactory* m_pDXGIFactory = nullptr;
		IDXGISwapChain* m_pSwapChain = nullptr;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11Texture2D* m_pDepthStencilBuffer;

		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11DepthStencilView* m_pDepthStencilView;
		//----------------------------------//
	};
}
#endif