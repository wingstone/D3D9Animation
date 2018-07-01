#include "SkinMesh.h"

bool g_useHardware = true;
char g_pathName[100];

//============����BoneHierachyLoader�ӿڣ����ڼ�����Ƥ����============//

HRESULT BoneHierachyLoader::CreateFrame(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
	XFRAME_EX* bone = new XFRAME_EX;
	
	AllocateName(Name, &bone->Name);

	D3DXMatrixIdentity(&bone->TransformationMatrix);
	D3DXMatrixIdentity(&bone->CombinedTransformationMatrix);

	*ppNewFrame = bone;

	return S_OK;
}

HRESULT BoneHierachyLoader::DestroyFrame(THIS_ LPD3DXFRAME pFrameToFree)
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

HRESULT BoneHierachyLoader::CreateMeshContainer(THIS_ LPCSTR Name, 
	CONST D3DXMESHDATA *pMeshData, 
	CONST D3DXMATERIAL *pMaterials, 
	CONST D3DXEFFECTINSTANCE *pEffectInstances, 
	DWORD NumMaterials, 
	CONST DWORD *pAdjacency, 
	LPD3DXSKININFO pSkinInfo, 
	LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
	XMESHCONTAINER_EX* boneMesh = new XMESHCONTAINER_EX;
	memset(boneMesh, 0, sizeof(XMESHCONTAINER_EX));

	//copy mesh
	boneMesh->OriginMesh = pMeshData->pMesh;					//���������mesh
	boneMesh->MeshData.pMesh = pMeshData->pMesh;		//������Ƥ���mesh���ȼ�������ľ�����Ƥ���
	boneMesh->MeshData.Type = pMeshData->Type;

	int NumFaces = pMeshData->pMesh->GetNumFaces();
	boneMesh->pAdjacency = new DWORD[NumFaces * 3];	//�洢ÿ����������������
	memcpy(boneMesh->pAdjacency, pAdjacency, sizeof(DWORD) * NumFaces * 3);

	pMeshData->pMesh->AddRef();			//�������ü�������ֹ��ɾ��

	//create material
	IDirect3DDevice9 * device = nullptr;
	pMeshData->pMesh->GetDevice(&device);

	boneMesh->NumMaterials = NumMaterials;
	for (UINT i =0 ; i < NumMaterials; i++)
	{
		D3DXMATERIAL mtrl;
		memcpy(&mtrl, &pMaterials[i], sizeof(D3DXMATERIAL));
		boneMesh->materials.push_back(mtrl.MatD3D);
		
		IDirect3DTexture9* texture = nullptr;
		if (mtrl.pTextureFilename)
		{
			char texName[100];
			strcpy_s(texName, g_pathName);
			strcat_s(texName, mtrl.pTextureFilename);
			D3DXCreateTextureFromFile(device, texName, &texture);
		}
		boneMesh->textures.push_back(texture);
	}

	//copy skin info
	if (pSkinInfo)
	{
		boneMesh->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();			//�������ü�������ֹ��ɾ��

		int numBones = pSkinInfo->GetNumBones();
		boneMesh->boneOffsetMaterices = new D3DXMATRIX[numBones];
		boneMesh->currentBoneMaterices = new D3DXMATRIX[numBones];

		for (int i = 0; i < numBones; i++)
		{
			boneMesh->boneOffsetMaterices[i] = *(boneMesh->pSkinInfo->GetBoneOffsetMatrix(i));
		}

		if (g_useHardware)
		{
			DWORD numBoneCombineEntries = 0;
			ID3DXBuffer* boneCombineTable = nullptr;
			pSkinInfo->ConvertToIndexedBlendedMesh(pMeshData->pMesh,
				D3DXMESH_MANAGED | D3DXMESH_WRITEONLY,
				35, nullptr, nullptr, nullptr, nullptr,
				&boneMesh->NumInfl,
				&numBoneCombineEntries,
				&boneCombineTable,
				&boneMesh->MeshData.pMesh);
			if (boneCombineTable)
			{
				boneCombineTable->Release();
				boneCombineTable = nullptr;
			}
		}
		else
		{
			//SAFE_RELEASE(boneMesh->MeshData.pMesh);
			pMeshData->pMesh->CloneMeshFVF(D3DXMESH_MANAGED,			//�������Ƥ��Ϣ�������ָ���Ķ����ʽ���±�����Ƥ����
				pMeshData->pMesh->GetFVF(),
				device,
				&boneMesh->MeshData.pMesh);
		}
		boneMesh->MeshData.pMesh->GetAttributeTable(nullptr, &boneMesh->numAttributesGroups);
		boneMesh->attributeTables = new D3DXATTRIBUTERANGE[boneMesh->numAttributesGroups];
		boneMesh->MeshData.pMesh->GetAttributeTable(boneMesh->attributeTables, nullptr);
	}

	*ppNewMeshContainer = boneMesh;

	return S_OK;
}

HRESULT BoneHierachyLoader::DestroyMeshContainer(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree)
{
	if (pMeshContainerToFree)
	{
		XMESHCONTAINER_EX* meshContainerToFree = (XMESHCONTAINER_EX*)pMeshContainerToFree;
		delete[] meshContainerToFree->attributeTables;
		delete[] meshContainerToFree->boneMaterixPtrs;
		delete[] meshContainerToFree->boneOffsetMaterices;
		delete[] meshContainerToFree->boneMaterixPtrs;

		for (UINT i = 0; i < meshContainerToFree->textures.size(); i++)
		{
			meshContainerToFree->textures[i]->Release();
		}

		SAFE_DELETE_ARRAY(meshContainerToFree->pAdjacency);

		SAFE_RELEASE(meshContainerToFree->pSkinInfo);
		SAFE_RELEASE(meshContainerToFree->OriginMesh);
		SAFE_RELEASE(meshContainerToFree->MeshData.pMesh);

		delete pMeshContainerToFree;
	}

	pMeshContainerToFree = nullptr;
	return S_OK;
}

//==================================skin mesh��������=========================//

SkinMesh::SkinMesh()
{
	_rootBone = nullptr;
	_pAnimationController = nullptr;
}

SkinMesh::~SkinMesh()
{
}

//�����ļ�������ʼ��
HRESULT SkinMesh::Load(IDirect3DDevice9* devicce, const char* pathName, const char* fileName)
{
	HRESULT hr = S_OK;

	strcpy_s(g_pathName, pathName);

	char meshName[100];
	strcpy_s(meshName, pathName);
	strcat_s(meshName, fileName);
	hr = D3DXLoadMeshHierarchyFromX(meshName,
		D3DXMESH_MANAGED,
		devicce,
		&_boneLoader,
		nullptr,
		&_rootBone,
		&_pAnimationController
	);
	if (FAILED(hr))
	{
		return hr;
	}

	//���ú���callback key�� animation controller
	ID3DXKeyframedAnimationSet *keyFrameAnimateSet = nullptr;
	UINT animSetNum = _pAnimationController->GetNumAnimationSets();
	UINT animSetId = 1;
	_pAnimationController->GetAnimationSet(animSetId, (ID3DXAnimationSet**)&keyFrameAnimateSet);

	//create callback key
	_key = new D3DXKEY_CALLBACK[1];
	_key[0].Time = float(keyFrameAnimateSet->GetPeriod() / keyFrameAnimateSet->GetSourceTicksPerSecond());
	MY_DEFINE_STRUCT *pSoundEnum = new MY_DEFINE_STRUCT;
	pSoundEnum->soundEnum = 0;
	_key[0].pCallbackData = (void*)pSoundEnum;

	bool useCompress = false;
	if (useCompress)
	{
		//ʹ��Compress ��animation set
		ID3DXBuffer* compressedData = NULL;
		keyFrameAnimateSet->Compress(D3DXCOMPRESS_DEFAULT, 0.5f, NULL, &compressedData);

		ID3DXCompressedAnimationSet* compressedAnimSet = NULL;
		D3DXCreateCompressedAnimationSet(keyFrameAnimateSet->GetName(),
			keyFrameAnimateSet->GetSourceTicksPerSecond(),
			keyFrameAnimateSet->GetPlaybackType(), compressedData,
			1, _key, &compressedAnimSet);
		compressedData->Release();

		_pAnimationController->UnregisterAnimationSet(keyFrameAnimateSet);

		_pAnimationController->RegisterAnimationSet(compressedAnimSet);

		_pAnimationController->SetTrackAnimationSet(0, compressedAnimSet);
		compressedAnimSet->Release();
	}
	else			//��ʱ����ʹ�ã���֪����ʲô����
	{
		//��ʹ��compress ��animation set
		keyFrameAnimateSet->SetCallbackKey(animSetId, _key);		//SetCallbackKey(animationIndex, keyArray);
		_pAnimationController->SetTrackAnimationSet(0, keyFrameAnimateSet);
	}

	//update matrix
	SetupBoneMatrixPointers((XFRAME_EX * )_rootBone);

	UpdataFrame();

	return hr;
}

//���¹�������
void SkinMesh::UpdataFrame( D3DXMATRIX* mat)
{
	UpdataFrame((XFRAME_EX *)_rootBone, mat);
}

//���¹�������
void SkinMesh::UpdataFrame()
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

	UpdataFrame(&mat);
}

//��ʱ����¶�����������
void SkinMesh::UpdateAnimate( float deltTime)
{
	_pAnimationController->AdvanceTime(deltTime, nullptr);
}

//������Ƥ����
void SkinMesh::Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	Render(device, (XFRAME_EX*)_rootBone, effect, model, view, projection);
}

//�ڴ�����
void SkinMesh::CleanUp()
{
	MY_DEFINE_STRUCT *pCallData = (MY_DEFINE_STRUCT*)_key->pCallbackData;
	SAFE_DELETE(pCallData);
	SAFE_DELETE_ARRAY(_key);

	SAFE_RELEASE(_pAnimationController);

	_boneLoader.DestroyFrame(_rootBone);
}

//���ظ�����
LPD3DXFRAME SkinMesh::ReturnRootBone()
{
	return _rootBone;
}

//����������������ָ��Ĺ�������
void SkinMesh::SetupBoneMatrixPointers(XFRAME_EX * bone)
{
	//�������е�bone����meshcontainer��Ӧ����ֻ��rootbone����meshcontainer
	//������meshcontainer��������ָ���bone��combine matrix
	if (bone->pMeshContainer)
	{
		XMESHCONTAINER_EX * boneMesh = (XMESHCONTAINER_EX *) bone->pMeshContainer;
		if (boneMesh->pSkinInfo)
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();

			boneMesh->boneMaterixPtrs = new D3DXMATRIX*[numBones];

			for (int i = 0; i < numBones; i++)
			{
				XFRAME_EX * bo = (XFRAME_EX*)D3DXFrameFind(_rootBone, boneMesh->pSkinInfo->GetBoneName(i));
			
				if (bo)
					boneMesh->boneMaterixPtrs[i] = &bo->CombinedTransformationMatrix;
				else
					boneMesh->boneMaterixPtrs[i] = nullptr;
			}
		}
	}

	if (bone->pFrameSibling)
		SetupBoneMatrixPointers((XFRAME_EX*)bone->pFrameSibling);
	if (bone->pFrameFirstChild)
		SetupBoneMatrixPointers((XFRAME_EX*)bone->pFrameFirstChild);
}

//���¹�������
void SkinMesh::UpdataFrame(XFRAME_EX * bone, D3DXMATRIX * parentMarix)
{
	if (bone)
		D3DXMatrixMultiply(&bone->CombinedTransformationMatrix, &bone->TransformationMatrix, parentMarix);

	if (bone->pFrameSibling)
		UpdataFrame((XFRAME_EX*)bone->pFrameSibling, parentMarix);

	if (bone->pFrameFirstChild)
		UpdataFrame((XFRAME_EX*)bone->pFrameFirstChild, &bone->CombinedTransformationMatrix);
};

//������Ƥ����
void SkinMesh::Render( IDirect3DDevice9* device, XFRAME_EX* bone, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	if (!bone)
		return;

	if (bone->pMeshContainer)
	{
		XMESHCONTAINER_EX* boneMesh = (XMESHCONTAINER_EX*)bone->pMeshContainer;

		if (boneMesh->pSkinInfo)
		{
			int numBones = boneMesh->pSkinInfo->GetNumBones();
			for (int i = 0; i < numBones; i++)
			{
				D3DXMatrixMultiply(&boneMesh->currentBoneMaterices[i], &boneMesh->boneOffsetMaterices[i], boneMesh->boneMaterixPtrs[i]);
			}

			if (g_useHardware)
			{
				effect->SetMatrixArray("MatPalette", boneMesh->currentBoneMaterices, numBones);
				effect->SetInt("NumBoneInfluences", boneMesh->NumInfl);

				effect->SetMatrix("Model", model);
				effect->SetMatrix("View", view);
				effect->SetMatrix("Projection", projection);

				effect->SetTechnique("TechSkin");

				for (UINT k =0;k<boneMesh->numAttributesGroups; k++)
				{
					UINT numPasses = 0;
					int mtrl = boneMesh->attributeTables[k].AttribId;
					effect->SetTexture("TexDiffuse", boneMesh->textures[mtrl]);

					effect->Begin(&numPasses, 0);
					for (UINT i = 0; i < numPasses; i++)
					{
						effect->BeginPass(i);
						boneMesh->MeshData.pMesh->DrawSubset(mtrl);
						effect->EndPass();
					}
					effect->End();
				}

			}
			else
			{
				BYTE * src = nullptr, *dst = nullptr;
				boneMesh->OriginMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&src);
				boneMesh->MeshData.pMesh->LockVertexBuffer(0, (void**)&dst);
				boneMesh->pSkinInfo->UpdateSkinnedMesh(boneMesh->currentBoneMaterices, nullptr, src, dst);
				boneMesh->MeshData.pMesh->UnlockVertexBuffer();
				boneMesh->OriginMesh->UnlockVertexBuffer();

				for (UINT i = 0; i < boneMesh->numAttributesGroups; i++)
				{
					int mtrl = boneMesh->attributeTables[i].AttribId;
					device->SetMaterial(&(boneMesh->materials[mtrl]));
					device->SetTexture(0, boneMesh->textures[mtrl]);
					boneMesh->MeshData.pMesh->DrawSubset(mtrl);
				}
			}

		}
		else
		{
			D3DXMATRIX model0;
			D3DXMatrixMultiply(&model0, &bone->CombinedTransformationMatrix, model);
			effect->SetMatrix("Model", &model0);
			effect->SetMatrix("View", view);
			effect->SetMatrix("Projection", projection);

			effect->SetTechnique("TechNoSkin");
			for (UINT k = 0; k < boneMesh->materials.size(); k++)
			{
				UINT numPasses = 0;
				effect->SetTexture("TexDiffuse", boneMesh->textures[k]);

				effect->Begin(&numPasses, 0);
				for (UINT i = 0; i < numPasses; i++)
				{
					effect->BeginPass(i);
					boneMesh->OriginMesh->DrawSubset(k);
					effect->EndPass();
				}
				effect->End();
			}
		}
	}

	if (bone->pFrameSibling)
		Render(device, (XFRAME_EX*)bone->pFrameSibling, effect, model, view, projection);
	if (bone->pFrameFirstChild)
		Render(device, (XFRAME_EX*)bone->pFrameFirstChild, effect, model, view, projection);
}
