#include "Application.h"
#include <Windows.h>

//程序运行入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE preInstance, PSTR cmdLine, int showCmd)
{
	Application app;
	if (FAILED(app.Initial(hInstance, true)))
	{
		MessageBox(nullptr, "Cann't initial application!", nullptr, 0);
		return 0;
	}

	MSG msg;
	memset(&msg, 0, sizeof(MSG));
	
	//消息循环
	DWORD startTime = GetTickCount();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DWORD t = GetTickCount();
			float deltTime = (t - startTime)*0.001f;

			if (InputCtrl::GetInstance()->IsEscapePressed())
				break;

			app.Update(deltTime);
			app.Render(deltTime);

			startTime = t;
		}
	}
	app.CleanUp();
	app.Quit();
	
	return (int)msg.wParam;
}