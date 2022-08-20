#include "pch.h"

#include <iostream>

#include "SDL_image.h"

#include "Texture.h"

//=== Constructors ===//
Texture::Texture(std::string filePath)
	: m_pSurface{ nullptr }
	, m_pTexture{ nullptr }
	, m_pResourceView{ nullptr }
{
	//Initializing for all textures
	m_pSurface = IMG_Load(filePath.c_str());
	if (!m_pSurface) std::cout << "Texture not loaded properly." << std::endl;
}

Texture::Texture(const char* filepath, ID3D11Device* pDevice)
	: Texture(filepath)
{
	//Extra initializing for DirectX textures
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);
	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);
	if (FAILED(result))
	{
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc{};
	SRVdesc.Format = desc.Format;
	SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVdesc.Texture2D.MipLevels = 1;
	result = pDevice->CreateShaderResourceView(m_pTexture, &SRVdesc, &m_pResourceView);
	if (FAILED(result))
	{
		return;
	}
}

//=== Destructor ===//
Texture::~Texture()
{
	SDL_FreeSurface(m_pSurface);

	if (m_pResourceView)
	{
		m_pResourceView->Release();
	}

	if (m_pTexture)
	{
		m_pTexture->Release();
	}
}

//=== Functions ===//
Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	Elite::RGBColor sample{};
	Uint8 r{}, g{}, b{};

	uint32_t x = uint32_t(uv.x * m_pSurface->w);
	uint32_t y = uint32_t(uv.y * m_pSurface->h);

	int index = x + (y * m_pSurface->w);

	Uint32* pixels = (Uint32*)m_pSurface->pixels;
	if (index < m_pSurface->h * m_pSurface->w && index >= 0)
	{
		SDL_GetRGB(pixels[index], m_pSurface->format, &r, &g, &b);
	}

	sample = Elite::RGBColor(float(r / 255.f), float(g / 255.f), float(b / 255.f));
	return sample;
}