#ifndef STATICMESH
#define STATICMESH

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include<Windows.h>
#include<math.h>

#include"Commen.h"
#include"Debug.h"

//用于加载静态网格
class StaticMesh
{
public:
	StaticMesh();
	StaticMesh(const StaticMesh& other);
	~StaticMesh();

	HRESULT Load(IDirect3DDevice9* device, const char* fileName);
	HRESULT Load(IDirect3DDevice9* device, const char* fileName, const char* texName);
	void Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

protected:
	bool _haveTex;
	ID3DXMesh * _pMesh;
	ID3DXBuffer* _pAdjacencyBuffer;
	IDirect3DTexture9* _pTex;
};

#endif // !STATICMESH
