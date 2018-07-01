#ifndef MORPHFACE
#define MORPHFACE

#include "StaticMesh.h"
#include "Face.h"

class Eye : public StaticMesh
{
public:
	Eye();
	Eye(const Eye& other);
	~Eye();

	void SetPos(D3DXVECTOR3 * pos);
	void Update(D3DXVECTOR2 * target);
	void Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);

private:
	D3DXVECTOR3 _position;
	D3DXMATRIX _modelMat;
};

class Head
{
public:
	Head();
	Head(const Head& other);
	~Head();

	HRESULT Load(IDirect3DDevice9* device, const char* faceFile, std::vector<std::string> *texFaceList, const char* eyeFile, std::vector<std::string> *texEyeList);
	void RandomHead();
	void Update(float deltTime);
	void RenderEye(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void RenderFace(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

private:
	Eye* _pEye1;
	Eye* _pEye2;
	int _eyeIndex;
	std::vector<IDirect3DTexture9 *>  _vecEyeTex;

	MorphFace* _pFace;
};


#endif // !MORPHMESH