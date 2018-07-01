#ifndef MORPHMESH
#define MORPHMESH

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include<Windows.h>
#include<math.h>

#include"Commen.h"
#include"Debug.h"

class MorphMesh
{
public:
	MorphMesh();
	MorphMesh(const MorphMesh& other);
	~MorphMesh();

	HRESULT Load(IDirect3DDevice9* device, const char* fileName1, const char* fileNeme2);
	void Update(float deltTime);
	void Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

protected:
	ID3DXMesh * _pMesh1, *_pMesh2;
	ID3DXBuffer* _pMaterialBuffer1;
	ID3DXBuffer* _pAdjacencyBuffer1;
	DWORD _numMaterial1;
	std::vector<IDirect3DTexture9*> _texVec;

	ID3DXMesh* _pFace;
};

class MultiMorphMesh
{
public:
	MultiMorphMesh();
	MultiMorphMesh(const MultiMorphMesh& other);
	~MultiMorphMesh();

	HRESULT Load(IDirect3DDevice9* device, std::vector<std::string> &stringList);
	void Update(float deltTime);
	void Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection);
	void CleanUp();

private:
	std::vector<ID3DXMesh *> _pMeshList;
	float * _poldWeightList;
	float * _pnewWeightList;
	float* _pweightList;

	size_t _meshNum;
	ID3DXMesh* _pFace;
	IDirect3DVertexDeclaration9* _pVertexDecl;
};


#endif // !MORPHMESH
