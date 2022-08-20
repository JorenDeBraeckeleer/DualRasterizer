#pragma once

#include <string>

#include "EMath.h"
#include "ERGBColor.h"

struct SDL_Surface;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

//=== Texture class ===//
class Texture final
{
public:
	//=== Constructor ===//
	Texture(std::string filePath);
	Texture(const char* filepath, ID3D11Device* pDevice);

	//=== Rule of five ===//
	~Texture();
	Texture(const Texture& texture) = delete;
	Texture(Texture&& texture) = delete;
	Texture& operator=(const Texture& texture) = delete;
	Texture& operator=(Texture&& texture) = delete;

	//=== Functions ===//
	ID3D11ShaderResourceView* GetResourceView() const { return m_pResourceView; }

	Elite::RGBColor Sample(const Elite::FVector2& uv) const;

private:
	//=== Variables ===//
	SDL_Surface* m_pSurface;
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pResourceView;
};