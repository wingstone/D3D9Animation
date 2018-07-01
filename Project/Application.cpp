#include "Application.h"
#include <string>

Application::Application()
{
	_device = nullptr;
	_mainWindow = nullptr;

	_effectCombine = nullptr;

	_solider = nullptr;

}

Application::~Application()
{
}

//窗口消息处理函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		break;

	case  WM_KEYDOWN:
		if(wParam == VK_ESCAPE)		//press esc键退出消息循环, 若使用DirectInput则会截取该消息
			PostQuitMessage(0);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

//应用初始化函数
HRESULT Application::Initial(HINSTANCE hInstance, bool windowed)
{
	//窗口初始化
	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "D3DWND";

	RegisterClass(&wc);
	_mainWindow = CreateWindow("D3DWND", "Animation Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, hInstance, 0);

	ShowWindow(_mainWindow, SW_SHOW);

	UpdateWindow(_mainWindow);

	//Direct3DDevice初始化
	IDirect3D9* d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9)
		return E_FAIL;

	D3DPRESENT_PARAMETERS present;
	present.BackBufferWidth = WINDOW_WIDTH;
	present.BackBufferHeight = WINDOW_HEIGHT;
	present.BackBufferFormat = D3DFMT_A8R8G8B8;
	present.BackBufferCount = 2;
	present.MultiSampleType = D3DMULTISAMPLE_NONE;
	present.MultiSampleQuality = 0;
	present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	present.hDeviceWindow = _mainWindow;
	present.Windowed = windowed;
	present.EnableAutoDepthStencil = true;
	present.AutoDepthStencilFormat = D3DFMT_D24S8;
	present.Flags = 0;
	present.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _mainWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present, &_device);
	if (!_device)
		return E_FAIL;

	//输入设备初始化
	if ( ! InputCtrl::GetInstance()->Initialize(hInstance, _mainWindow, WINDOW_WIDTH, WINDOW_HEIGHT))
		return E_FAIL;

	//加载模型网格
	HRESULT hr = S_OK;
	hr = InitialMesh();

	//set View port
	D3DVIEWPORT9 viewPort;
	viewPort.X = 0;
	viewPort.Y = 0;
	viewPort.Width = WINDOW_WIDTH;
	viewPort.Height = WINDOW_HEIGHT;
	viewPort.MinZ = 0;
	viewPort.MaxZ = 1;
	_device->SetViewport(&viewPort);

	//initial debug file
	if(!Debug::OpenFile("Debug.txt"))
		return E_FAIL;

	return hr;
}

//加载模型网格
HRESULT Application::InitialMesh()
{
	HRESULT hr = S_OK;
	ID3DXBuffer* errorBuffer;

#pragma region Character
	_solider = new Character();
	hr = _solider->LoadCharacterMesh(_device, "Resource\\soldier\\", "soldier.x");
	if (FAILED(hr))
	{
		MessageBox(_mainWindow, " Can't Load soldierMesh", "Load error", MB_OK);
		return hr;
	}

	std::vector<std::string> texFacelist;
	for (size_t i = 0; i < 8; i++)
	{
		std::string faceTex = std::string("Resource\\FaceFactory\\face0") + std::to_string(i + 1) + std::string(".jpg");
		texFacelist.push_back(faceTex);
	}

	std::vector<std::string> texEyeList;
	for (size_t i = 0; i < 5; i++)
	{
		std::string eyeTex = std::string("Resource\\FaceFactory\\eye0") + std::to_string(i + 1) + std::string(".jpg");
		texEyeList.push_back(eyeTex);
	}
	hr = _solider->LoadHeadMesh(_device, "Resource\\FaceFactory\\factory_face.x", &texFacelist, "Resource\\FaceFactory\\eye.x", &texEyeList);
	if (FAILED(hr))
	{
		MessageBox(_mainWindow, " Can't Load headMesh", "Load error", MB_OK);
		return hr;
	}

	//load effect
	hr = D3DXCreateEffectFromFile(_device, "Resource\\Effect\\Combine.fx", nullptr, nullptr, D3DXSHADER_DEBUG, nullptr, &_effectCombine, &errorBuffer);
	if (FAILED(hr))
	{
		MessageBox(_mainWindow, (char*)errorBuffer->GetBufferPointer(), "effect error", MB_OK);
		errorBuffer->Release();
		return hr;
	}
	if (errorBuffer)
	{
		MessageBox(_mainWindow, (char*)errorBuffer->GetBufferPointer(), "effect error", MB_OK);
		errorBuffer->Release();
	}
#pragma endregion

	return S_OK;
}

//每帧的内容更新
void Application::Update(float deltTime)
{
	//更新设备输入
	InputCtrl::GetInstance()->Frame();

	//更新模型矩阵
	_solider->Update(deltTime);
}

//每帧的绘制
void Application::Render(float deltTime)	//单位为秒		一些网格更新与渲染可以分离，一些不可以分离
{
	_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0xFFBBFFFF, 1, 0);

	if (SUCCEEDED(_device->BeginScene()))
	{
		D3DXMATRIX model;
		D3DXMatrixTranslation(&model, 0.0f, 0.0f, 0.0f);

		D3DXMATRIX view;
		D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 1.5f, -1.0f);
		D3DXVECTOR3 at = D3DXVECTOR3(0.0f, 1.5f,0.0f);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&view, &eye, &at, &up);

		D3DXMATRIX projection;
		D3DXMatrixPerspectiveFovLH(&projection, D3DX_PI / 3.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);

#pragma region Character
		_solider->Render(_device, _effectCombine, &model, &view, &projection);

#pragma endregion

		_device->EndScene();
		_device->Present(0, 0, 0, 0);
	}
}

//内存清理
void Application::CleanUp()
{
	_effectCombine->Release();

	_solider->CleanUp();
	SAFE_DELETE(_solider);

	InputCtrl::GetInstance()->ShutDown();

	_device->Release();

	Debug::CloseFile();
}

void Application::Quit()
{
	
}

void Application::DeviceLost()
{

}

void Application::DeviceGained()
{

}