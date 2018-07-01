#ifndef SKINMESH
#define SKINMESH

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <fstream>

#include "Commen.h"
#include "Debug.h"

//for callback key
struct MY_DEFINE_STRUCT
{
	int soundEnum;
};

class CallbackHandler : public ID3DXAnimationCallbackHandler
{
public:
	HRESULT CALLBACK HandleCallback(THIS_ UINT Track, LPVOID pCallbackData)
	{
		MY_DEFINE_STRUCT *u = (MY_DEFINE_STRUCT *)pCallbackData;
		if (u->soundEnum == 0)
		{
			PlaySound(TEXT("SystemStart"), NULL, SND_ALIAS| SND_ASYNC);
			Debug::OutputMessage("Bang!");
		}	

		return S_OK;
	}
};

//扩展骨架
struct XFRAME_EX : public D3DXFRAME
{
	/*
	D3DXFRAME {
	LPSTR Name;
	D3DXMATRIX TransformationMatrix;
	LPD3DXMESHCONTAINER pMeshContainer;
	D3DXFRAME* pFrameSibling;
	D3DXFRAME* pFrameChild;
	}
	*/
	D3DXMATRIX CombinedTransformationMatrix;
};

//扩展mesh container
struct XMESHCONTAINER_EX : public D3DXMESHCONTAINER
{
	/*
	D3DXMESHCONTAINER {
		LPSTR Name;
		D3DXMESHDATA MeshData;
		LPD3DXMATERIAL pMaterials;
		LPD3DXEFFECTINSTANCE pEffects;
		DWORD NumMaterials;
		DWORD *pAdjacency;
		LPD3DXSKININFO pSkinInfo;
		D3DXMESHCONTAINER * pNextMeshContainer;
	*/
	ID3DXMesh* OriginMesh;
	std::vector<D3DMATERIAL9> materials;
	std::vector<IDirect3DTexture9*> textures;

	DWORD numAttributesGroups;		//骨骼属性组，即子网格数量
	DWORD NumInfl;
	D3DXATTRIBUTERANGE* attributeTables;
	D3DXMATRIX** boneMaterixPtrs;
	D3DXMATRIX* boneOffsetMaterices;
	D3DXMATRIX* currentBoneMaterices;
};

//重载 allocate hierachy接口
class BoneHierachyLoader : public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);

	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR Name,
		CONST D3DXMESHDATA *pMeshData,
		CONST D3DXMATERIAL *pMaterials,
		CONST D3DXEFFECTINSTANCE *pEffectInstances,
		DWORD NumMaterials,
		CONST DWORD *pAdjacency,
		LPD3DXSKININFO pSkinInfo,
		LPD3DXMESHCONTAINER *ppNewMeshContainer);

	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);

	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree);
};

//skinmesh 类
class SkinMesh
{
public:
	SkinMesh();
	SkinMesh(const SkinMesh& other);
	~SkinMesh();

	HRESULT Load(IDirect3DDevice9* devicce, const char* pathName, const char* fileName);

	void UpdataFrame();
	void UpdataFrame( D3DXMATRIX* mat);
	void UpdataFrame(XFRAME_EX * bone, D3DXMATRIX * parentMarix);

	void UpdateAnimate(float deltTime);

	void Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void Render(IDirect3DDevice9* device, XFRAME_EX* bone, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);

	void CleanUp();

	LPD3DXFRAME ReturnRootBone();

private:
	void SetupBoneMatrixPointers(XFRAME_EX * bone);


	D3DXFRAME* _rootBone;
	ID3DXAnimationController* _pAnimationController;
	BoneHierachyLoader _boneLoader;

	D3DXKEY_CALLBACK *_key;
	CallbackHandler _callbackHandler;
};

#endif