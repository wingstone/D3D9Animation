#ifndef FACE
#define FACE

#include "SkinMesh.h"

//重载 allocate hierachy接口，用于加载表情
class FaceHierachyLoader : public ID3DXAllocateHierarchy
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

//MorphSkinMesh
class MorphFace
{
public:
	MorphFace();
	MorphFace(const MorphFace& other);
	~MorphFace();

	HRESULT Load(IDirect3DDevice9* devicce, const char* meshName, std::vector<std::string> *texNameList);
	void RandomFace();
	void Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

private:
	void ExtractMesh(D3DXFRAME* rootBone);
	void SetStreamSources(IDirect3DDevice9* device);
	void UpdateFaceExpress();

private:
	ID3DXMesh* _baseFace;
	ID3DXMesh* _blinkFace;
	int _emotionIndex;
	std::vector<ID3DXMesh*> _vecEmotionFace;
	int _speechIndex1, _speechIndex2;
	std::vector<ID3DXMesh*> _vecSpeechFace;
	FaceHierachyLoader _faceLoader;

	IDirect3DVertexDeclaration9* _pVertexDecl;
	int _faceIndex;
	std::vector<IDirect3DTexture9 *>  _vecTex;
	float _morphWeight[4];
};

#endif