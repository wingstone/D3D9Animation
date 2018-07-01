#include "InputCtrl.h"

InputCtrl::InputCtrl()
{
	_directInput = nullptr;
	_keyBoard = nullptr;
	_mouse = nullptr;
}

InputCtrl::~InputCtrl()
{
}

//输入设备的初始化
bool InputCtrl::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT hr;

	_screenWidth = screenWidth;
	_screenHeight = screenHeight;
	_mouseX = 0;
	_mouseY = 0;

	hr = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&_directInput, nullptr);
	if (FAILED(hr))
		return false;

	hr = _directInput->CreateDevice(GUID_SysKeyboard, &_keyBoard, nullptr);
	if (FAILED(hr))
		return false;
	hr = _keyBoard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
		return false;
	hr = _keyBoard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(hr))
		return false;
	hr = _keyBoard->Acquire();
	if (FAILED(hr))
		return false;

	hr = _directInput->CreateDevice(GUID_SysMouse, &_mouse, nullptr);
	if (FAILED(hr))
		return false;
	hr = _mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr))
		return false;
	hr = _mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(hr))
		return false;
	hr = _mouse->Acquire();
	if (FAILED(hr))
		return false;

	return true;
}

//内存释放
void InputCtrl::ShutDown()
{
	if (_keyBoard)
	{
		_keyBoard->Unacquire();
		_keyBoard->Release();
		_keyBoard = nullptr;
	}

	if (_mouse)
	{
		_mouse->Unacquire();
		_mouse->Release();
		_mouse = nullptr;
	}

	SAFE_RELEASE(_directInput);
}

//每帧输入的更新
bool InputCtrl::Frame()
{
	bool result;

	result = ReadKeyBoard();
	if (!result)
		return false;

	result = ReadMouse();
	if (!result)
		return false;

	ProcessInput();

	return true;
}

//获取键盘输入
bool InputCtrl::ReadKeyBoard()
{
	HRESULT hr;

	hr = _keyBoard->GetDeviceState(sizeof(_keyBoadState), (LPVOID)&_keyBoadState);
	if (FAILED(hr))
	{
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)		//处理失焦的情况
			_keyBoard->Acquire();
		else
			return false;
	}

	return true;
}

//获取鼠标输入
bool InputCtrl::ReadMouse()
{
	HRESULT hr;

	hr = _mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&_mouseState);
	if (FAILED(hr))
	{
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)		//处理失焦的情况
			_mouse->Acquire();
		else
			return false;
	}

	return true;
}

//从鼠标坐标系转窗口坐标系
void InputCtrl::ProcessInput()
{
	_mouseX += _mouseState.lX;
	_mouseY += _mouseState.lY;

	if (_mouseX < 0)_mouseX = 0;
	if (_mouseY < 0)_mouseY = 0;

	if (_mouseX > _screenWidth) _mouseX = _screenWidth;
	if (_mouseY > _screenHeight) _mouseY = _screenHeight;
}

//判断是否按下ESC键
bool InputCtrl::IsEscapePressed()
{
	if (_keyBoadState[DIK_ESCAPE] & 0x80)
		return true;

	return false;
}

//判断是否按下W键
bool InputCtrl::IsKeyWPressed()
{
	if (_keyBoadState[DIK_W] & 0x80)
		return true;

	return false;
}

//判断是否按下A键
bool InputCtrl::IsKeyAPressed()
{
	if (_keyBoadState[DIK_A] & 0x80)
		return true;

	return false;
}

//判断是否按下S键
bool InputCtrl::IsKeySPressed()
{
	if (_keyBoadState[DIK_S] & 0x80)
		return true;

	return false;
}

//判断是否按下D键
bool InputCtrl::IsKeyDPressed()
{
	if (_keyBoadState[DIK_D] & 0x80)
		return true;

	return false;
}

//判断空格键是否按下
bool InputCtrl::IsSpacePressed()
{
	if (_keyBoadState[DIK_SPACE] & 0x80)
		return true;

	return false;
}

//获取窗口上鼠标坐标
void InputCtrl::GetMouseLocation(int* x, int* y)
{
	*x = _mouseX;
	*y = _mouseY;
}

//初始化单例
InputCtrl * InputCtrl::_instance = new InputCtrl();

//获取唯一单例
InputCtrl* InputCtrl::GetInstance()
{
	return _instance;
}