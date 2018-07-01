#include "MorphMesh.h"

static bool USINGGPU = true;

//MorphMesh
MorphMesh::MorphMesh()
{
	_pMesh1 = nullptr;
	_pMesh1 = nullptr;
	_pMaterialBuffer1 = nullptr;
	_pAdjacencyBuffer1 = nullptr;
	_numMaterial1 = 0;
}

MorphMesh::~MorphMesh()
{}

//加载相互转换的两个网格文件
HRESULT MorphMesh::Load(IDirect3DDevice9* device, const char* fileName1, const char* fileNeme2)
{
	HRESULT hr = S_OK;

	hr = D3DXLoadMeshFromX(fileName1, D3DXMESH_MANAGED, device, &_pAdjacencyBuffer1, &_pMaterialBuffer1, nullptr, &_numMaterial1, &_pMesh1);
	if (FAILED(hr))
	{
		MessageBox(nullptr, "can't load MorphMesh", ".x file error", MB_OK);
		return hr;
	}
	hr = D3DXLoadMeshFromX(fileNeme2, D3DXMESH_MANAGED, device, nullptr, nullptr, nullptr, nullptr, &_pMesh2);
	if (FAILED(hr))
	{
		MessageBox(nullptr, "can't load MorphMesh", ".x file error", MB_OK);
		return hr;
	}
	_pMesh1->CloneMeshFVF(D3DXMESH_MANAGED, _pMesh1->GetFVF(), device, &_pFace);

	//D3DXMATERIAL * mtrls = (D3DXMATERIAL*)_pMaterialBuffer1->GetBufferPointer();

	//for (UINT i = 0; i<_numMaterial1; i++)
	//{
	//	if (mtrls[i].pTextureFilename)
	//	{
	//		IDirect3DTexture9* tex;
	//		D3DXCreateTextureFromFile(device, mtrls[i].pTextureFilename, &tex);
	//		_texVec.push_back(tex);
	//	}
	//}

	return hr;
}

//使用CPU情况下进行网格插值
void MorphMesh::Update(float deltTime)
{
	BYTE *target1, *target2, *face;
	_pMesh1->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&target1);
	_pMesh2->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&target2);
	_pFace->LockVertexBuffer(0, (LPVOID*)&face);

	static float blendWeight = 0;
	if(blendWeight <= 1.0f)
		blendWeight += deltTime / 2.0f;
	for (DWORD i = 0; i < _pFace->GetNumVertices(); i++)
	{
		D3DXVECTOR3 *t1 = (D3DXVECTOR3 *)target1;
		D3DXVECTOR3 *t2 = (D3DXVECTOR3 *)target2;
		D3DXVECTOR3 *f = (D3DXVECTOR3 *)face;

		*f = *t1 * (1.0f - blendWeight ) + *t2 * blendWeight;

		target1 += _pMesh1->GetNumBytesPerVertex();
		target2 += _pMesh2->GetNumBytesPerVertex();
		face += _pFace->GetNumBytesPerVertex();
	}

	_pMesh1->UnlockVertexBuffer();
	_pMesh2->UnlockVertexBuffer();
	_pFace->UnlockVertexBuffer();
}

//绘制插值后的网格文件
void MorphMesh::Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	effect->SetMatrix("Model", model);
	effect->SetMatrix("View", view);
	effect->SetMatrix("Projection", projection);

	effect->SetTechnique("Tech");

	UINT numPasses = 0;
	effect->Begin(&numPasses, 0);
	for (UINT i = 0; i < numPasses; i++)
	{
		effect->BeginPass(i);

		//for (UINT j = 0; j < _numMaterial1; j++)
		//{
		//	effect->SetTexture("diff", _texVec[j]);
		//	_pFace->DrawSubset(j);
		//}

		_pFace->DrawSubset(0);

		effect->EndPass();
	}
	effect->End();

}

//内存清理
void MorphMesh::CleanUp()
{
	for (size_t i = 0; i < _texVec.size(); i++)
	{
		SAFE_RELEASE(_texVec[i]);
	}
	SAFE_RELEASE(_pMaterialBuffer1);
	SAFE_RELEASE(_pAdjacencyBuffer1);
	SAFE_RELEASE(_pMesh1);
	SAFE_RELEASE(_pMesh2);
	SAFE_RELEASE(_pFace);
}

//MultiMorphMesh
MultiMorphMesh::MultiMorphMesh()
{
	_pFace = nullptr;
	_poldWeightList = nullptr;
	_pnewWeightList = nullptr;
	_pweightList = nullptr;
}


MultiMorphMesh::~MultiMorphMesh()
{}

//加载一系列可插值网格文件，最多五个
HRESULT MultiMorphMesh::Load(IDirect3DDevice9* device, std::vector<std::string> &stringList)
{
	HRESULT hr = S_OK;

	ID3DXMesh* pMesh;
	for (size_t i = 0; i < stringList.size(); i++)
	{
		hr = D3DXLoadMeshFromX( stringList[i].c_str(), D3DXMESH_MANAGED, device, nullptr, nullptr, nullptr, nullptr, &pMesh);
		if (FAILED(hr))
		{
			return hr;
		}
		_pMeshList.push_back(pMesh);
	}

	_meshNum = stringList.size();
	_poldWeightList = new float[stringList.size()];
	_pnewWeightList = new float[stringList.size()];
	_pweightList = new float[stringList.size()];

	_pMeshList[0]->CloneMeshFVF(D3DXMESH_MANAGED, _pMeshList[0]->GetFVF(), device, &_pFace);

	//设置顶点声明
	D3DVERTEXELEMENT9 elementDecl[] =
	{
		//stream0
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		//stream1
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
		{ 1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1 },
		{ 1, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },

		//stream2
		{ 2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
		{ 2, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 2 },
		{ 2, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },

		//stream3
		{ 3, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 3 },
		{ 3, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 3 },
		{ 3, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },

		//stream4
		{ 4, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 4 },
		{ 4, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 4 },
		{ 4, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4 },

		D3DDECL_END()
	};

	hr = device->CreateVertexDeclaration(elementDecl, &_pVertexDecl);
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}

//更新各个差值网格的权重
void MultiMorphMesh::Update(float deltTime)
{
	//初始化权重
	static float blendWeight = 0.0f;
	static bool isFirst = true;

	//初始化weights
	if (isFirst)
	{
		float oldLen = 0.0f;
		float newLen = 0.0f;
		for (size_t i = 0; i < _meshNum; i++)
		{
			_poldWeightList[i] = (rand() % 1000) / 1000.0f;
			_poldWeightList[i] = max(_poldWeightList[i] * 2.0f - 1.0f, 0);
			_pnewWeightList[i] = (rand() % 1000) / 1000.0f;
			_pnewWeightList[i] = max(_pnewWeightList[i] * 2.0f - 1.0f, 0);

			oldLen += _poldWeightList[i];
			newLen += _pnewWeightList[i];
		}

		for (size_t i = 0; i < _meshNum; i++)
		{
			_poldWeightList[i] = _poldWeightList[i] / oldLen;
			_pnewWeightList[i] = _pnewWeightList[i] / newLen;
		}

		isFirst = false;
	}

	//更新weights
	if (blendWeight <= 1.0f)
	{
		blendWeight += deltTime / 1.0f;		//此处1.0f代表没过1秒，变换一次表情
		for (size_t i = 0; i < _meshNum; i++)
		{
			_pweightList[i] = _poldWeightList[i] * ( 1.0f - blendWeight ) + _pnewWeightList[i] * blendWeight;
		}
	}
	else
	{
		blendWeight = 0.0f;
		float newLen = 0.0f;

		for (size_t i = 0; i < _meshNum; i++)
		{
			_poldWeightList[i] = _pnewWeightList[i];
			_pnewWeightList[i] = (rand() % 1000) / 1000.0f;
			_pnewWeightList[i] = max(_pnewWeightList[i] * 2.0f - 1.0f, 0);

			newLen += _pnewWeightList[i];
		}

		for (size_t i = 0; i < _meshNum; i++)
		{
			_pnewWeightList[i] = _pnewWeightList[i] / newLen;
		}
	}

	//更新网格数据
	if (!USINGGPU)
	{
		BYTE **target = new BYTE*[_pMeshList.size()], *face;
		for (size_t i = 0; i < _pMeshList.size(); i++)
		{
			_pMeshList[i]->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&target[i]);
		}

		_pFace->LockVertexBuffer(0, (LPVOID*)&face);


		for (DWORD i = 0; i < _pFace->GetNumVertices()-1; i++)
		{
			D3DXVECTOR3 *f = (D3DXVECTOR3 *)face;
			*f = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

			for (size_t j = 0; j < _pMeshList.size(); j++)
			{
				D3DXVECTOR3 *t1 = (D3DXVECTOR3 *)target[j];
				*f += *t1 * _pweightList[j];

				target[j] += _pFace->GetNumBytesPerVertex();
			}

			face += _pFace->GetNumBytesPerVertex();
		}


		for (size_t i = 0; i < _pMeshList.size(); i++)
		{
			_pMeshList[i]->UnlockVertexBuffer();
		}

		_pFace->UnlockVertexBuffer();

		SAFE_DELETE_ARRAY(target);
	}
	else
	{
		//只需前面更新权重即可
	}
}

//使用GPU进行网格插值，并绘制网格
void MultiMorphMesh::Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	if (!USINGGPU)
	{
		effect->SetMatrix("Model", model);
		effect->SetMatrix("View", view);
		effect->SetMatrix("Projection", projection);

		effect->SetTechnique("Tech");

		UINT numPasses = 0;
		effect->Begin(&numPasses, 0);
		for (UINT i = 0; i < numPasses; i++)
		{
			effect->BeginPass(i);

			_pFace->DrawSubset(0);

			effect->EndPass();
		}
		effect->End();
	}
	else
	{
		device->SetVertexDeclaration(_pVertexDecl);

		//设置顶点缓存，索引缓存
		DWORD vSize = D3DXGetFVFVertexSize(_pMeshList[0]->GetFVF());
		
		IDirect3DVertexBuffer9* vertexBuffer = nullptr;
		for (size_t i = 0; i < _meshNum; i++)
		{
			_pMeshList[i]->GetVertexBuffer(&vertexBuffer);
			device->SetStreamSource(i, vertexBuffer, 0, vSize);
		}

		IDirect3DIndexBuffer9* indexBuffer = nullptr;
		_pMeshList[0]->GetIndexBuffer(&indexBuffer);
		device->SetIndices(indexBuffer);

		//设置tech 数据
		effect->SetMatrix("Model", model);
		effect->SetMatrix("View", view);
		effect->SetMatrix("Projection", projection);

		effect->SetTechnique("TechFace");
		effect->SetFloatArray("Weights", _pweightList, 5);

		//render
		UINT numPasses = 0;
		effect->Begin(&numPasses, 0);
		for (UINT i = 0; i < numPasses; i++)
		{
			effect->BeginPass(i);

			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _pMeshList[0]->GetNumVertices(), 0, _pMeshList[0]->GetNumFaces());

			effect->EndPass();
		}
		effect->End();
	}

}

//内存清理
void MultiMorphMesh::CleanUp()
{
	SAFE_DELETE_ARRAY(_poldWeightList);
	SAFE_DELETE_ARRAY(_pnewWeightList);
	SAFE_DELETE_ARRAY(_pweightList);

	SAFE_RELEASE(_pVertexDecl);
	SAFE_RELEASE(_pFace);
	for (size_t i = 0; i < _meshNum; i++)
	{
		SAFE_RELEASE(_pMeshList[i]);
	}
}