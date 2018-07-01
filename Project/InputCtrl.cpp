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

//�����豸�ĳ�ʼ��
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

//�ڴ��ͷ�
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

//ÿ֡����ĸ���
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

//��ȡ��������
bool InputCtrl::ReadKeyBoard()
{
	HRESULT hr;

	hr = _keyBoard->GetDeviceState(sizeof(_keyBoadState), (LPVOID)&_keyBoadState);
	if (FAILED(hr))
	{
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)		//����ʧ�������
			_keyBoard->Acquire();
		else
			return false;
	}

	return true;
}

//��ȡ�������
bool InputCtrl::ReadMouse()
{
	HRESULT hr;

	hr = _mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&_mouseState);
	if (FAILED(hr))
	{
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)		//����ʧ�������
			_mouse->Acquire();
		else
			return false;
	}

	return true;
}

//���������ϵת��������ϵ
void InputCtrl::ProcessInput()
{
	_mouseX += _mouseState.lX;
	_mouseY += _mouseState.lY;

	if (_mouseX < 0)_mouseX = 0;
	if (_mouseY < 0)_mouseY = 0;

	if (_mouseX > _screenWidth) _mouseX = _screenWidth;
	if (_mouseY > _screenHeight) _mouseY = _screenHeight;
}

//�ж��Ƿ���ESC��
bool InputCtrl::IsEscapePressed()
{
	if (_keyBoadState[DIK_ESCAPE] & 0x80)
		return true;

	return false;
}

//�ж��Ƿ���W��
bool InputCtrl::IsKeyWPressed()
{
	if (_keyBoadState[DIK_W] & 0x80)
		return true;

	return false;
}

//�ж��Ƿ���A��
bool InputCtrl::IsKeyAPressed()
{
	if (_keyBoadState[DIK_A] & 0x80)
		return true;

	return false;
}

//�ж��Ƿ���S��
bool InputCtrl::IsKeySPressed()
{
	if (_keyBoadState[DIK_S] & 0x80)
		return true;

	return false;
}

//�ж��Ƿ���D��
bool InputCtrl::IsKeyDPressed()
{
	if (_keyBoadState[DIK_D] & 0x80)
		return true;

	return false;
}

//�жϿո���Ƿ���
bool InputCtrl::IsSpacePressed()
{
	if (_keyBoadState[DIK_SPACE] & 0x80)
		return true;

	return false;
}

//��ȡ�������������
void InputCtrl::GetMouseLocation(int* x, int* y)
{
	*x = _mouseX;
	*y = _mouseY;
}

//��ʼ������
InputCtrl * InputCtrl::_instance = new InputCtrl();

//��ȡΨһ����
InputCtrl* InputCtrl::GetInstance()
{
	return _instance;
}