#include "StaticMesh.h"

//StaticMesh
StaticMesh::StaticMesh()
{
	_pMesh = nullptr;
	_pAdjacencyBuffer = nullptr;
	_haveTex = false;
	_pTex = nullptr;
}

StaticMesh::~StaticMesh()
{}

//加载静态网格文件
HRESULT StaticMesh::Load(IDirect3DDevice9* device, const char* fileName1)
{
	HRESULT hr = S_OK;

	hr = D3DXLoadMeshFromX(fileName1, D3DXMESH_MANAGED, device, &_pAdjacencyBuffer, nullptr, nullptr, nullptr, &_pMesh);
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}

//加载静态网格文件和纹理文件
HRESULT StaticMesh::Load(IDirect3DDevice9* device, const char* fileName, const char* texName)
{
	HRESULT hr = S_OK;

	hr = D3DXLoadMeshFromX(fileName, D3DXMESH_MANAGED, device, nullptr, nullptr, nullptr, nullptr, &_pMesh);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = D3DXCreateTextureFromFile(device, texName, &_pTex);
	if (FAILED(hr))
		return hr;

	_haveTex = true;

	return hr;
}

//绘制静态网格
void StaticMesh::Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	effect->SetMatrix("Model", model);
	effect->SetMatrix("View", view);
	effect->SetMatrix("Projection", projection);

	effect->SetTechnique("TechTex");

	if (_haveTex)
		effect->SetTexture("TexDiffuse", _pTex);

	UINT numPasses = 0;
	effect->Begin(&numPasses, 0);
	for (UINT i = 0; i < numPasses; i++)
	{
		effect->BeginPass(i);

		_pMesh->DrawSubset(0);

		effect->EndPass();
	}
	effect->End();

}

//清理内存
void StaticMesh::CleanUp()
{
	SAFE_RELEASE(_pAdjacencyBuffer);
	SAFE_RELEASE(_pMesh);
	SAFE_RELEASE(_pTex);
}
