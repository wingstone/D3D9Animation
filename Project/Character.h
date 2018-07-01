#ifndef CHARACTER
#define CHARACTER

#include "SkinMesh.h"
#include "Head.h"
#include "InverseKinematic.h"
#include "InputCtrl.h"

class Character
{
public:
	Character();
	Character(const Character& other);
	~Character();

	HRESULT LoadCharacterMesh(IDirect3DDevice9* devicce, const char* pathName, const char* characFile);
	HRESULT LoadHeadMesh(IDirect3DDevice9* device, const char* faceFile, std::vector<std::string> *texFaceList, const char* eyeFile, std::vector<std::string> *texEyeList);
	void Update(float deltTime);
	void Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void Render(IDirect3DDevice9* device, XFRAME_EX* bone, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

private:
	SkinMesh *_pBody;
	Head *_pHead;

	D3DXMATRIX _headModel;
	InverseKinematic_Head _inverseKinematic;
};

#endif // !CHARACTER
