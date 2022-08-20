#include "pch.h"
//#undef main

//------------//
#ifdef _DEBUG
#include "vld.h"
#endif
//------------//

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - De Braeckeleer Joren",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	//------------------//
	//=== FPS toggle ===//
	bool showFPS{ true };
	//------------------//

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:

				//------------------------------------------------------------------------------------//
				//=== Keybindings ===//
				if (e.key.keysym.scancode == SDL_SCANCODE_E ||
					e.key.keysym.scancode == SDL_SCANCODE_R ||
					e.key.keysym.scancode == SDL_SCANCODE_C ||
					e.key.keysym.scancode == SDL_SCANCODE_F ||
					e.key.keysym.scancode == SDL_SCANCODE_T ||
					e.key.keysym.scancode == SDL_SCANCODE_N ||
					e.key.keysym.scancode == SDL_SCANCODE_Z) pRenderer->InfoKeys(e.key.keysym.scancode);

				if (e.key.keysym.scancode == SDL_SCANCODE_O)
					pRenderer->InfoText();

				if (e.key.keysym.scancode == SDL_SCANCODE_P)
					showFPS = !showFPS;
				//------------------------------------------------------------------------------------//

				break;
			}
		}

		//--------- Render ---------
		pRenderer->Render();
		//----------------------------------------//
		//=== Update ===//
		pRenderer->Update(pTimer->GetElapsed());
		pRenderer->SetElapsedTime(pTimer->GetElapsed());
		//----------------------------------------//

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (showFPS) std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

	}
	pTimer->Stop();

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}