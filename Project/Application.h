#ifndef APPLICATION
#define APPLICATION

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

#include "InputCtrl.h"
#include "SkinMesh.h"
#include "MorphMesh.h"
#include "Head.h"
#include "Character.h"
#include "Commen.h"

class Application
{
public:
	Application();
	~Application();

	HRESULT Initial(HINSTANCE hInstance, bool windowed);
	void Update(float DeltTime);
	void Render(float deltTime);
	void CleanUp();
	void Quit();

	void DeviceLost();
	void DeviceGained();

private:
	HRESULT InitialMesh();

private:
	HWND _mainWindow;
	D3DPRESENT_PARAMETERS _parsent;
	IDirect3DDevice9* _device;
	bool _deviceLost;

	Character* _solider;
	ID3DXEffect* _effectCombine;

};

#endif // !APPLICATION

#pragma once
