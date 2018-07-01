#ifndef INPUTCLASS_H
#define INPUTCLASS_H

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <windows.h>
#include "Commen.h"

//��������� ����ģʽ��ֻ��ʹ��SingletonObj���󣬲����Լ�����
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

	static InputCtrl* GetInstance();		//��ȡ����

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

	//����״̬�洢
	unsigned char _keyBoadState[256];
	DIMOUSESTATE _mouseState;

	int _screenWidth, _screenHeight;
	int _mouseX, _mouseY;	//��������ϵ�����Ĵ������꣨0-screenWidth, 0-screenHeight��

	//����
	static InputCtrl *_instance;
};

#endif
