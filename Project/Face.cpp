#include "Face.h"

//============重载BoneHierachyLoader接口，用来加载脸型============//

HRESULT FaceHierachyLoader::CreateFrame(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
	D3DXFRAME* bone = new D3DXFRAME;

	AllocateName(Name, &bone->Name);

	*ppNewFrame = bone;

	return S_OK;
}

HRESULT FaceHierachyLoader::DestroyFrame(THIS_ LPD3DXFRAME pFrameToFree)
{
	if (pFrameToFree)
	{
		if (pFrameToFree->Name)
		{
			delete[] pFrameToFree->Name;
		}

		delete pFrameToFree;
	}

	pFrameToFree = nullptr;

	return S_OK;
}

HRESULT FaceHierachyLoader::CreateMeshContainer(THIS_ LPCSTR Name,
	CONST D3DXMESHDATA *pMeshData,
	CONST D3DXMATERIAL *pMaterials,
	CONST D3DXEFFECTINSTANCE *pEffectInstances,
	DWORD NumMaterials,
	CONST DWORD *pAdjacency,
	LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
	D3DXMESHCONTAINER* boneMesh = new D3DXMESHCONTAINER;
	memset(boneMesh, 0, sizeof(D3DXMESHCONTAINER));

	//copy mesh
	boneMesh->MeshData.pMesh = pMeshData->pMesh;
	boneMesh->MeshData.Type = pMeshData->Type;

	pMeshData->pMesh->AddRef();			//增加引用计数，防止被删掉

	*ppNewMeshContainer = boneMesh;

	return S_OK;
}

HRESULT FaceHierachyLoader::DestroyMeshContainer(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree)
{
	if (pMeshContainerToFree)
	{
		SAFE_RELEASE(pMeshContainerToFree->MeshData.pMesh);
		delete pMeshContainerToFree;
	}

	pMeshContainerToFree = nullptr;
	return S_OK;
}

//=========================MorphFace定义==============================//

MorphFace::MorphFace()
{
	_baseFace = nullptr;
	_blinkFace = nullptr;
	_faceIndex = 0;
	_pVertexDecl = nullptr;

	_emotionIndex = 0;
	_speechIndex1 = 0;
	_speechIndex2 = 0;
}

MorphFace::~MorphFace()
{
}

//加载文件，并初始化
HRESULT MorphFace::Load(IDirect3DDevice9* devicce, const char* meshName, std::vector<std::string> *texNameList)
{
	HRESULT hr = S_OK;

	//load mesh
	D3DXFRAME * rootBone;
	hr = D3DXLoadMeshHierarchyFromX(meshName,
		D3DXMESH_MANAGED,
		devicce,
		&_faceLoader,
		nullptr,
		&rootBone,
		nullptr
	);
	if (FAILED(hr))
	{
		return hr;
	}

	//store mesh
	ExtractMesh(rootBone);

	_faceLoader.DestroyFrame(rootBone);

	//load texture
	for (size_t i = 0; i < texNameList->size(); i++)
	{
		IDirect3DTexture9* pTex;
		hr = D3DXCreateTextureFromFile(devicce, (*texNameList)[i].c_str(), &pTex);
		_vecTex.push_back(pTex);
		if (FAILED(hr))
		{
			return hr;
		}
	}


	//create element decleration
	D3DVERTEXELEMENT9 elementDecl[] =
	{
		//stream0	base face
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },

		//stream1	blink face
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
		{ 1, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1 },

		//stream2	emotion face
		{ 2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
		{ 2, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 2 },

		//stream3	speech1 face
		{ 3, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 3 },
		{ 3, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 3 },

		//stream4 speech2 face
		{ 4, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 4 },
		{ 4, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 4 },

		//stream5	skin face
		{ 5, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 5 },		//why 12?
		{ 5, 16, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 5 },

		D3DDECL_END()
	};

	hr = devicce->CreateVertexDeclaration(elementDecl, &_pVertexDecl);
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}

//将不同脸型存储于不同XMESH中
void MorphFace::ExtractMesh(D3DXFRAME* rootBone)
{
	if (rootBone->pMeshContainer)
	{
		if (rootBone->Name)
		{
			ID3DXMesh* pMesh = rootBone->pMeshContainer->MeshData.pMesh;
			if (strcmp(rootBone->Name, "Base") == 0)
			{
				_baseFace = pMesh;
				pMesh->AddRef();
			}
			if (strcmp(rootBone->Name, "Blink") == 0)
			{
				_blinkFace = pMesh;
				pMesh->AddRef();
			}
			if (strcmp(rootBone->Name, "Emotion") == 0)
			{
				_vecEmotionFace.push_back( pMesh );
				pMesh->AddRef();
			}
			if (strcmp(rootBone->Name, "Speech") == 0)
			{
				_vecSpeechFace.push_back( pMesh );
				pMesh->AddRef();
			}
		}
	}

	if (rootBone->pFrameSibling)
		ExtractMesh(rootBone->pFrameSibling);
	if (rootBone->pFrameFirstChild)
		ExtractMesh(rootBone->pFrameFirstChild);
}

//随机切换脸部纹理
void MorphFace::RandomFace()
{
	_faceIndex = rand() % 8;
}

//设置绘制网格数据源
void MorphFace::SetStreamSources(IDirect3DDevice9* device)
{
	IDirect3DVertexBuffer9* vb = nullptr;
	_baseFace->GetVertexBuffer(&vb);
	DWORD stride = _baseFace->GetNumBytesPerVertex();
	device->SetStreamSource(0, vb, 0, stride);

	_blinkFace->GetVertexBuffer(&vb);
	device->SetStreamSource(1, vb, 0, stride);

	_vecEmotionFace[_emotionIndex]->GetVertexBuffer(&vb);
	device->SetStreamSource(2, vb, 0, stride);

	_vecSpeechFace[_speechIndex1]->GetVertexBuffer(&vb);
	device->SetStreamSource(3, vb, 0, stride);

	_vecSpeechFace[_speechIndex2]->GetVertexBuffer(&vb);
	device->SetStreamSource(4, vb, 0, stride);

	IDirect3DIndexBuffer9* ib = nullptr;
	_baseFace->GetIndexBuffer(&ib);
	device->SetIndices(ib);
}

//更新脸部插值权重
void MorphFace::UpdateFaceExpress()
{
	static DWORD startTime = timeGetTime();
	DWORD time = timeGetTime() - startTime;

	//0: blink, 1: emotion, 2,3: speech
	//处理眨眼
	if (time % 4500 < 250) {		//睁眼
		_morphWeight[0] = 1.0f - time % 4500 / 250.0f;
	}
	else if (time % 4500 < 4250) {
		_morphWeight[0] = 0;
	}
	else		//闭眼
	{
		_morphWeight[0] = (time % 4500 - 4250) / 250.0f;
	}

	//切换表情
	if (time % 2000 > 1990)
		_emotionIndex = rand() % _vecEmotionFace.size();

	if (time % 2000 < 500) {
		_morphWeight[1] = time % 2000 / 800.0f;		//变化表情
	}
	else if (time % 2000 < 1800)			//维持表情
	{
		_morphWeight[1] = 1.0f;
	}
	else			//恢复原样
	{
		_morphWeight[1] = 1.0f - (time % 2000 - 1800) / 200.0f;
	}

	//切换语言
	if (time % 1000 == 500)
		_speechIndex1 = _speechIndex2;
	if (time % 1000 == 999)
		_speechIndex2 = rand() % _vecSpeechFace.size();
	
	//处理语言权重
	if (time % 1000 < 500) {
		_morphWeight[2] = 1.0f - time % 1000 / 500.0f;
		_morphWeight[3] = time % 1000 / 500.0f;
	}
	else
	{
		_morphWeight[2] = (time % 1000 - 500) / 500.0f;
		_morphWeight[3] = (1000 - time % 1000) / 500.0f;
	}

}

//绘制脸型
void MorphFace::Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	device->SetVertexDeclaration(_pVertexDecl);

	UpdateFaceExpress();

	SetStreamSources(device);

	//render
	effect->SetMatrix("Model", model);
	effect->SetMatrix("View", view);
	effect->SetMatrix("Projection", projection);
	effect->SetFloatArray("MorphWeights", _morphWeight, 4);

	effect->SetTexture("TexDiffuse", _vecTex[_faceIndex]);

	effect->SetTechnique("TechMorphSkin");
	UINT numPasses = 0;
	effect->Begin(&numPasses, 0);
	for (UINT i = 0; i < numPasses; i++)
	{
		effect->BeginPass(i);
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, _baseFace->GetNumVertices(), 0, _baseFace->GetNumFaces());
		effect->EndPass();
	}
	effect->End();

	//reset stream
	for (int i = 0; i < 6; i++)
	{
		device->SetStreamSource(i, nullptr, 0, 0);
	}
}

//内存清理
void MorphFace::CleanUp()
{
	SAFE_RELEASE(_pVertexDecl);

	SAFE_RELEASE(_baseFace);
	SAFE_RELEASE(_blinkFace);

	for (size_t i = 0; i < _vecEmotionFace.size(); i++)
	{
		SAFE_RELEASE(_vecEmotionFace[i]);
	}

	for (size_t i = 0; i < _vecSpeechFace.size(); i++)
	{
		SAFE_RELEASE(_vecSpeechFace[i])
	}
	
	for (size_t i = 0; i < _vecTex.size(); i++)
	{
		SAFE_RELEASE(_vecTex[i]);
	}
}