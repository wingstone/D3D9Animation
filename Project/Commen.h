#ifndef COMMEN
#define COMMEN

#include <Windows.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

//�ڴ��ͷź�
#define SAFE_DELETE_ARRAY(p) if(p){delete[] p; p = nullptr;}
#define  SAFE_DELETE(p) if(p){delete p; p = nullptr;}
#define SAFE_RELEASE(p) if(p){p->Release(); p = nullptr;}

//��pNewName��������
HRESULT AllocateName(LPCSTR Name, LPSTR* pNewName);

#endif
