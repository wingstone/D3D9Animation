#ifndef INPUTCLASS_H
#define INPUTCLASS_H

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <windows.h>
#include "Commen.h"

//输入控制类 单例模式，只能使用SingletonObj对象，不能自己声明
class InputCtrl
{
public:
	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void ShutDown();
	bool Frame();

	bool IsEscapePressed();
	bool IsKeyWPressed();
	bool IsKeyAPressed();
	bool IsKeySPressed();
	bool IsKeyDPressed();
	bool IsSpacePressed();
	void GetMouseLocation(int* x, int* y);

	static InputCtrl* GetInstance();		//获取单例

private:
	InputCtrl();
	InputCtrl(const InputCtrl& input);
	~InputCtrl();

	bool ReadKeyBoard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8* _directInput;
	IDirectInputDevice8* _keyBoard;
	IDirectInputDevice8* _mouse;

	//输入状态存储
	unsigned char _keyBoadState[256];
	DIMOUSESTATE _mouseState;

	int _screenWidth, _screenHeight;
	int _mouseX, _mouseY;	//窗口坐标系上鼠标的窗口坐标（0-screenWidth, 0-screenHeight）

	//单例
	static InputCtrl *_instance;
};

#endif
