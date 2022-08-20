#include "pch.h"

#include <iostream>

#include "ERenderer.h"
#include "EOBJParser.h"

#include "Vertex.h"
#include "Material.h"
#include "DiffuseMaterial.h"
#include "TexturedMaterial.h"
#include "Texture.h"
#include "Mesh.h"
#include "Triangle.h"

Elite::Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	//Initialize
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//--------------------------------------------------------------------------------------------------------------------------------------------//
	//Initialize DirectX pipeline
	if (SUCCEEDED(InitializeDirectX()))
	{
		m_IsInitialized = true;
	}

	//=== Keybindings ===//
	InfoText();

	//=== Camera ===//
	float aspectRatio{ float(m_Width) / float(m_Height) };
	m_pCamera = std::make_unique<Camera>(m_UsingSoftware, aspectRatio, Elite::FPoint3(0.0f, 0.0f, 0.0f), Elite::FVector3(0.0f, 0.0f, -1.0f), 45.0f);

	//=== DepthBuffer ===//
	m_pDepthBufferPixels = new float[size_t(m_Width) * size_t(m_Height)];

	//=== Mesh ===//
	InitVehicle();
	//--------------------------------------------------------------------------------------------------------------------------------------------//
}

void Elite::Renderer::Render()
{
	if (m_UsingSoftware)
	{
		RenderSoftware();
	}
	else
	{
		if (!m_IsInitialized)
		{
			return;
		}

		RenderHardware();
	}
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//=== Initialize ===//
void Elite::Renderer::InitVehicle()
{
	//- Main Mesh -//
	//Initialize textures
	m_pTexture = new Texture{ "Resources/vehicle_diffuse.png", m_pDevice };
	m_pNormal = new Texture{ "Resources/vehicle_normal.png", m_pDevice };
	m_pSpecular = new Texture{ "Resources/vehicle_specular.png", m_pDevice };
	m_pGloss = new Texture{ "Resources/vehicle_gloss.png", m_pDevice };

	//Initialize materials
	m_pVehicleEffect = new TexturedMaterial(m_pDevice, L"Resources/LambertPhongShader.fx", "LambertPhongImprovedTechnique");

	//Parse object
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	Elite::ParseOBJ("Resources/vehicle.obj", vertices, indices);

	//Push object for software rendering
	m_pSoftwareMeshes.push_back(new Mesh{ vertices, indices, Mesh::PrimitiveTopology::TriangleList });
	for (Mesh* pMesh : m_pSoftwareMeshes) for (Triangle* pTriangle : pMesh->GetTriangles()) m_pTriangles.push_back(pTriangle);

	//Push object for hardware rendering (change z-pos -> left handed coordinate system)
	for (Vertex& vertex : vertices) { vertex.position.z *= -1; vertex.normal.z *= -1; vertex.tangent.z *= -1; } //Invert z (left handed coordinate system)
	m_pHardwareMeshes.push_back(new Mesh{ m_pDevice, vertices, indices, m_pVehicleEffect, m_pTexture, m_pNormal, m_pSpecular, m_pGloss });

	//- Fire Mesh -//
	//Initialize textures
	m_pFireDiffuse = new Texture{ "Resources/fireFX_diffuse.png", m_pDevice };

	//Initialize materials
	m_pFireEffect = new DiffuseMaterial(m_pDevice, L"Resources/AlphaShader.fx", "FilterTechnique");

	//Parse object
	std::vector<Vertex> verticesFire{};
	std::vector<uint32_t> indicesFire{};

	Elite::ParseOBJ("Resources/fireFX.obj", verticesFire, indicesFire);

	//Push object for rendering (separate for toggle)
	m_pFireMesh = new Mesh{ m_pDevice, verticesFire, indicesFire, m_pFireEffect, m_pFireDiffuse };
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//--- Create Device and Device context, using hardware acceleration ---//
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result))
	{
		return result;
	}

	//Create DXGI Factory to create SwapChain based on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
	{
		return result;
	}

	//--- Create SwapChain Descriptor ---//
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//--- Get the handle HWND from the SDL Backbuffer ---//
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//--- Create SwapChain and hook it into the handle of the SDL window ---//
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//--- Create the Depth/Stencil Buffer and view ---//
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//--- Describe the resource view for our Depth/Stencil Buffer ---//
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//--- Create the actual resource and the matching resource view ---//
	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}

//=== Pipeline ===//
//- Software -//
void Elite::Renderer::RenderSoftware()
{
	SDL_LockSurface(m_pBackBuffer);

	//------------------------------------------------------------------------------------------------------------------------------------------//
	//=== Fill depthbuffer and backbuffer ===//
	std::fill(m_pDepthBufferPixels, m_pDepthBufferPixels + size_t(m_Width) * size_t(m_Height), FLT_MAX);
	std::fill(m_pBackBufferPixels, m_pBackBufferPixels + size_t(m_Width) * size_t(m_Height), GetSDL_ARGBColor(Elite::RGBColor(0.1f, 0.1f, 0.1f)));

	//=== Loop over triangles ===//
	for (Triangle* pTriangle : m_pTriangles)
	{
		//=== Projection stage -> transforming vertices ===//
		ProjectionStage(pTriangle, m_pCamera->GetViewToWorld(), m_pCamera->GetFov(), m_Width, m_Height, m_pCamera->GetPosition());

		//=== Rasterization stage + PixelShading stage -> calculating correct values to sample a pixel color ===//
		RasterizationStage(pTriangle, m_Width, m_Height, m_pDepthBufferPixels);
	}
	//------------------------------------------------------------------------------------------------------------------------------------------//

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Elite::Renderer::ProjectionStage(Triangle* pTriangle, const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height, const Elite::FPoint3& cameraPos)
{
	//If no rotation is needed make elapsed time zero so the object doesn't move
	if (!m_IsRotating) m_ElapsedTime = 0.f;

	//First part of vertex transformation
	ModelToWorld(pTriangle, cameraPos, m_ElapsedTime);

	//Second part of vertex transformation
	ModelToNDC(pTriangle, cameraToWorld, fovAngle, width, height);
}

void Elite::Renderer::ModelToWorld(Triangle* pTriangle, const Elite::FPoint3& cameraPos, float elapsedTime)
{
	//First part of vertex transformation
	pTriangle->ModelToWorld(cameraPos, m_WorldMatrix);
}

void Elite::Renderer::ModelToNDC(Triangle* pTriangle, const Elite::FMatrix4& cameraToWorld, float fovAngle, uint32_t width, uint32_t height)
{
	//Second part of vertex transformation
	pTriangle->ModelToNDC(cameraToWorld, fovAngle, width, height);
}

void Elite::Renderer::RasterizationStage(Triangle* pTriangle, uint32_t width, uint32_t height, float* depthBuffer)
{
	//Frustum culling check
	if (FrustumCulling(pTriangle))
	{
		//Skip whole triangle if triangle is out of frame
		return;
	}

	//Third part of vertex transformation
	NDCToScreen(pTriangle, width, height);

	//Calculate bounding box
	std::vector<Elite::FPoint2> boundingBox = pTriangle->GetBoundingBox(float(width), float(height));

	//Loop over pixels in bounding box
	for (uint32_t r = uint32_t(boundingBox[0].y); r < uint32_t(boundingBox[1].y); ++r)
	{
		for (uint32_t c = uint32_t(boundingBox[0].x); c < uint32_t(boundingBox[1].x); ++c)
		{
			//Initialize current pixel and weights
			const Elite::FPoint2 pixel{ (float)c, (float)r };
			float weight0{}, weight1{}, weight2{};

			//Pixel in triangle (hit) check
			if (PixelInTriangle(pTriangle, pixel, weight0, weight1, weight2))
			{
				//Initialize wInterpolated
				float wInterpolated{};

				//Depth check and calculation
				if (Depth(pTriangle, depthBuffer[c + (r * width)], wInterpolated, weight0, weight1, weight2))
				{
					//Depth buffer toggle
					if (!m_IsDepthBufferColor)
					{
						//Initialize interpolated values
						Elite::FVector2 uvInterpolated{};
						Elite::FVector3 normalInterpolated{};
						Elite::FVector3 tangentInterpolated{};
						Elite::FVector3 viewDirectionInterpolated{};
						Elite::RGBColor colorInterpolated{};

						//Attribute interpolation
						AttributeInterpolation(pTriangle, wInterpolated, weight0, weight1, weight2, uvInterpolated, normalInterpolated, tangentInterpolated, viewDirectionInterpolated, colorInterpolated);

						//Calculate final color
						Elite::RGBColor finalColor = PixelShadingStage(uvInterpolated, normalInterpolated, tangentInterpolated, viewDirectionInterpolated, colorInterpolated);
						finalColor.MaxToOne();

						//Draw on back buffer
						uint32_t uColor{ GetSDL_ARGBColor(finalColor) };
						m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(uColor >> 16),
							static_cast<uint8_t>(uColor >> 8),
							static_cast<uint8_t>(uColor));
					}
					else
					{
						//Calculate depth color
						float depth{ Elite::Remap(m_pDepthBufferPixels[c + (r * m_Width)], 1.f, 0.985f) };
						Elite::RGBColor depthColor{ depth, depth, depth };
						depthColor.MaxToOne();

						//Draw on back buffer
						uint32_t uColor{ GetSDL_ARGBColor(depthColor) };
						m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(uColor >> 16),
							static_cast<uint8_t>(uColor >> 8),
							static_cast<uint8_t>(uColor));
					}
				}
			}
		}
	}
}

bool Elite::Renderer::FrustumCulling(Triangle* pTriangle)
{
	//Frustum culling check
	return pTriangle->FrustumCulling();
}

void Elite::Renderer::NDCToScreen(Triangle* pTriangle, uint32_t width, uint32_t height)
{
	//Third part of vertex transformation
	pTriangle->NDCToScreen(width, height);
}

bool Elite::Renderer::PixelInTriangle(Triangle* pTriangle, const Elite::FPoint2& pixel, float& weight0, float& weight1, float& weight2)
{
	//Pixel in triangle check
	return pTriangle->PixelInTriangle(pixel, weight0, weight1, weight2, m_CullMode);
}

bool Elite::Renderer::Depth(Triangle* pTriangle, float& depthBufferPixel, float& wInterpolated, const float weight0, const float weight1, const float weight2)
{
	//Depth check and calculation
	return pTriangle->Depth(depthBufferPixel, wInterpolated, weight0, weight1, weight2);
}

void Elite::Renderer::AttributeInterpolation(Triangle* pTriangle, const float wInterpolated, const float weight0, const float weight1, const float weight2, Elite::FVector2& uvInterpolated,
	Elite::FVector3& normalInterpolated, Elite::FVector3& tangentInterpolated, Elite::FVector3& viewDirectionInterpolated, Elite::RGBColor& colorInterpolated)
{
	//Attribute interpolation
	pTriangle->AttributeInterpolation(pTriangle, wInterpolated, weight0, weight1, weight2, uvInterpolated, normalInterpolated, tangentInterpolated, viewDirectionInterpolated, colorInterpolated);
}

Elite::RGBColor Elite::Renderer::PixelShadingStage(const Elite::FVector2& uvInterpolated, const Elite::FVector3& normalInterpolated, const Elite::FVector3& tangentInterpolated,
	const Elite::FVector3& viewDirectionInterpolated, const Elite::RGBColor& colorInterpolated)
{
	//Implementation with 'backwards compatibility' for colors and textures without normals etc.
	if (m_pTexture)
	{
		//Initialize light
		const Elite::FVector3 lightDirection{ Elite::GetNormalized(Elite::FVector3(0.577f, -0.577f, -0.577f)) };
		const Elite::RGBColor lightColor{ 1.0f, 1.0f, 1.0f };
		const float lightIntensity{ 7.0f };

		//Calculate ambient color
		const Elite::RGBColor ambientColor = Ambient();

		//First stage of pixelshading
		Elite::FVector3 newNormal{};
		Elite::RGBColor diffuseColor{};
		if (m_pNormal)
		{
			if (m_IsNormalMapping)
			{
				//Normal mapping and diffuse color
				newNormal = NormalMapping(uvInterpolated, normalInterpolated, tangentInterpolated);
				diffuseColor = Diffuse(uvInterpolated, newNormal, lightDirection, lightColor, lightIntensity);
			}
			else
			{
				//Diffuse color
				diffuseColor = Diffuse(uvInterpolated, normalInterpolated, lightDirection, lightColor, lightIntensity);
			}
		}
		else
		{
			diffuseColor = m_pTexture->Sample(uvInterpolated);
		}

		//Second stage of pixelshading
		if (m_pSpecular && m_pGloss)
		{
			//Initialize shininess and specularReflectance
			const float shininess{ 25.0f };
			float specularReflectance{};

			//Specular
			const Elite::RGBColor specularColor = Specular(viewDirectionInterpolated, uvInterpolated, newNormal, -lightDirection, shininess, specularReflectance);

			return ambientColor + diffuseColor + specularColor;
		}

		return ambientColor + diffuseColor;
	}
	else
	{
		return colorInterpolated;
	}
}
Elite::FVector3 Elite::Renderer::NormalMapping(const Elite::FVector2& uvInterpolated, const Elite::FVector3& normalInterpolated, const Elite::FVector3& tangentInterpolated)
{
	//Sample normals
	const Elite::RGBColor normalMapSample = m_pNormal->Sample(uvInterpolated);

	//Calculate biNormal, tangentSpaceAxis and newNormal
	const Elite::FVector3 biNormal{ Elite::Cross(tangentInterpolated, normalInterpolated) };
	const Elite::FMatrix3 tangentSpaceAxis{ tangentInterpolated, biNormal, normalInterpolated };
	const Elite::FVector3 newNormal{ Elite::GetNormalized(tangentSpaceAxis * Elite::FVector3(2.0f * normalMapSample.r - 1.0f, 2.0f * normalMapSample.g - 1.0f, 2.0f * normalMapSample.b - 1.0f)) };

	return newNormal;
}
Elite::RGBColor Elite::Renderer::Diffuse(const Elite::FVector2& uvInterpolated, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection, const Elite::RGBColor& lightColor,
	const float lightIntensity)
{
	//Sample texture color
	Elite::RGBColor diffuseMapSample = m_pTexture->Sample(uvInterpolated);

	//Calculate irradiance with observedArea
	const float observedArea{ std::max(Elite::Dot(newNormal, -lightDirection), 0.0f) };
	const Elite::RGBColor irradiance{ lightColor * lightIntensity * observedArea };

	return irradiance * (diffuseMapSample / float(E_PI));
}
Elite::RGBColor Elite::Renderer::Specular(const Elite::FVector3& viewDirectionInterpolated, const Elite::FVector2& uvInterpolated, const Elite::FVector3& newNormal, const Elite::FVector3& lightDirection,
	const float shininess, float& specularReflectance)
{
	//Sample glossiness
	const Elite::RGBColor specularColor = m_pSpecular->Sample(uvInterpolated);
	//Sample specular
	const float phongExponent = m_pGloss->Sample(uvInterpolated).r * shininess;

	//Calculate reflection, angle and phong
	const Elite::FVector3 reflect{ Elite::Reflect(-lightDirection, newNormal) };
	const float angle = std::clamp(Elite::Dot(reflect, viewDirectionInterpolated), 0.f, 1.f);
	const float phongSpecularReflect{ 1.0f * powf(angle, phongExponent) };
	const Elite::RGBColor specular{ specularColor * phongSpecularReflect };

	return specular;
}
Elite::RGBColor Elite::Renderer::Ambient()
{
	//Extra ambient color
	Elite::RGBColor ambientColor{ 0.025f, 0.025f, 0.025f };
	return ambientColor;
}

//- Hardware -//
void Elite::Renderer::RenderHardware()
{
	//Clear Buffers
	Elite::RGBColor clearColor(0.1f, 0.1f, 0.1f);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	const Elite::FMatrix4 viewMatrix{ m_pCamera->GetWorldToView() };
	const Elite::FMatrix4 projectionMatrix{ m_pCamera->GetProjectionMatrix() };

	//Render
	for (Mesh* pMesh : m_pHardwareMeshes)
	{
		pMesh->Render(m_pDeviceContext, m_Filter, m_CullMode, m_WorldMatrix, viewMatrix, projectionMatrix);
	}

	if (m_ShowFireMesh)
	{
		m_pFireMesh->Render(m_pDeviceContext, m_Filter, Triangle::CullMode::Static, m_WorldMatrix, viewMatrix, projectionMatrix);
	}

	//Present
	m_pSwapChain->Present(0, 0);
}

//=== Update ===//
void Elite::Renderer::Update(float elapsedSec)
{
	m_pCamera->Update(m_UsingSoftware, elapsedSec);

	//Translation
	Elite::FMatrix4 translationMatrix = Elite::MakeTranslation(Elite::FVector3(0.0f, 0.0f, -50.0f));

	if (!m_UsingSoftware)
	{
		translationMatrix[3][2] *= -1;
	}

	//Rotation
	Elite::FMatrix4 rotationMatrix = Elite::FMatrix4::Identity();
	const float angle{ 45.0f };

	if (m_IsRotating)
	{
		m_RotationAngleR += Elite::ToRadians(angle * elapsedSec);
		m_RotationAngleL += Elite::ToRadians(-angle * elapsedSec);
	}

	if (m_UsingSoftware)
	{
		rotationMatrix = Elite::MakeRotationY(m_RotationAngleR);
	}
	else
	{
		rotationMatrix = Elite::MakeRotationY(m_RotationAngleL);
	}

	//Scaling
	const Elite::FMatrix4 scaleMatrix = Elite::FMatrix4::Identity();

	m_WorldMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}

//=== Keybindings ===//
void Elite::Renderer::InfoKeys(SDL_Scancode key)
{
	//Toggle render mode
	if (key == SDL_SCANCODE_E)
	{
		m_UsingSoftware = !m_UsingSoftware;
		std::cout << "Render mode toggled" << std::endl;
	}

	//Toggle for rotation
	if (key == SDL_SCANCODE_R)
	{
		m_IsRotating = !m_IsRotating;
		std::cout << "Rotation toggled" << std::endl;
	}

	//Toggle culling mode
	if (key == SDL_SCANCODE_C)
	{
		if (m_CullMode == Triangle::CullMode::BackFaceCulling)
		{
			m_CullMode = Triangle::CullMode::FrontFaceCulling;

			std::cout << "Now front-face culling" << std::endl;
		}
		else if (m_CullMode == Triangle::CullMode::FrontFaceCulling)
		{
			m_CullMode = Triangle::CullMode::NoCulling;

			std::cout << "Now no culling" << std::endl;
		}
		else if (m_CullMode == Triangle::CullMode::NoCulling)
		{
			m_CullMode = Triangle::CullMode::BackFaceCulling;

			std::cout << "Now back-face culling" << std::endl;
		}
	}

	if (!m_UsingSoftware)
	{
		//Toggle filter
		if (key == SDL_SCANCODE_F)
		{
			if (m_Filter == Mesh::Filter::Point)
			{
				m_Filter = Mesh::Filter::Linear;

				std::cout << "Now linear filtering" << std::endl;
			}
			else if (m_Filter == Mesh::Filter::Linear)
			{
				m_Filter = Mesh::Filter::Anisotropic;

				std::cout << "Now anisotropic filtering" << std::endl;
			}
			else if (m_Filter == Mesh::Filter::Anisotropic)
			{
				m_Filter = Mesh::Filter::Point;

				std::cout << "Now point filtering" << std::endl;
			}
		}

		//Toggle fire
		if (key == SDL_SCANCODE_T)
		{
			m_ShowFireMesh = !m_ShowFireMesh;
			std::cout << "Fire mesh toggled" << std::endl;
		}
	}
	else
	{
		////Optional
		//Toggle for depth buffer color view
		if (key == SDL_SCANCODE_Z)
		{
			m_IsDepthBufferColor = !m_IsDepthBufferColor;
			std::cout << "Depthbuffer toggled" << std::endl;
		}
	}
}

void Elite::Renderer::InfoText()
{
	std::cout <<
		"---------------------------------------------------------------------------------------\n" <<
		"|  \t\t\t\t\t\t\t\t\t\t      |\n" <<
		"|   GameGraphicsProgramming - Exam - DualRasterizer - Joren De Braeckeleer - 2GD06E   |\n" <<
		"|  \t\t\t\t\t\t\t\t\t\t      |\n" <<
		"---------------------------------------------------------------------------------------\n" <<
		"\t\t\t\t\t\t\t\t\t    P: Show FPS\n\n"
		"CONTROLS (O):\n" <<
		"    Mouse:\n" <<
		"\t-Rotate:\n\t    RMB + MouseMove-X: Yaw\n\t    RMB + MouseMove-Y: Pitch\n\t    LMB + MouseMove-X: Yaw\n" <<
		"\t-Moving:\n\t    LMB + MouseMove - Y: Forward / Backward\n\t    LMB + RMB + MouseMove - Y: Up / Down\n" <<
		"    KeyBoard:\n" <<
		"\t-Moving:\n\t    W: Forward  /  S: Backward\n\t    A: Left  /  D: Right\n" <<
		"\t-Rendering:\n\t    R: Toggle rotate\n\t    C: Toggle culling mode\n\t    E: Toggle system\n" <<
		"\t    -Software only: \n\t\tZ: Toggle depth buffer\n" <<
		"\t    -Hardware only: \n\t\tT: Toggle fire mesh\n\t\tF: Toggle filter" <<
		std::endl;
}

//=== Deleting ===//
Elite::Renderer::~Renderer()
{
	//Textures
	delete m_pTexture;
	m_pTexture = nullptr;
	delete m_pNormal;
	m_pNormal = nullptr;
	delete m_pSpecular;
	m_pSpecular = nullptr;
	delete m_pGloss;
	m_pGloss = nullptr;

	//- Software -//
	//Meshes (triangles)
	for (Mesh* pMesh : m_pSoftwareMeshes)
	{
		delete pMesh;
		pMesh = nullptr;
	}

	//Depth buffer
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;

	//- Hardware -//
	delete m_pFireDiffuse;
	m_pFireDiffuse = nullptr;

	for (Mesh* pMesh : m_pHardwareMeshes)
	{
		delete pMesh;
		pMesh = nullptr;
	}

	delete m_pFireMesh;
	m_pFireMesh = nullptr;

	delete m_pVehicleEffect;
	m_pVehicleEffect = nullptr;
	delete m_pFireEffect;
	m_pFireEffect = nullptr;

	m_pDepthStencilView->Release();
	m_pRenderTargetView->Release();

	m_pDepthStencilBuffer->Release();
	m_pRenderTargetBuffer->Release();

	m_pSwapChain->Release();

	m_pDXGIFactory->Release();

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	m_pDevice->Release();
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//