#include "Character.h"

Character::Character()
{
	_pHead = nullptr;
}

Character::~Character()
{}

//���ؽ�ɫ��Ƥ����
HRESULT Character::LoadCharacterMesh(IDirect3DDevice9* devicce, const char* pathName, const char* characFile)
{
	HRESULT hr = S_OK;
	_pBody = new SkinMesh();
	hr = _pBody->Load(devicce, pathName, characFile);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

//����ͷ����ֵ����
HRESULT Character::LoadHeadMesh(IDirect3DDevice9* device, const char* faceFile, std::vector<std::string> *texFaceList, const char* eyeFile, std::vector<std::string> *texEyeList)
{
	HRESULT hr = S_OK;
	_pHead = new Head();
	hr = _pHead->Load(device, faceFile, texFaceList, eyeFile, texEyeList);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

//���¶����еĹ�������
void Character::Update(float deltTime)
{
	_pBody->UpdateAnimate(deltTime);
	_pBody->UpdataFrame();

	//��ʼ��Kinematic,���Ŷ������У�CombineMat�ᷢ���仯
	XFRAME_EX* rootBone = (XFRAME_EX*)_pBody->ReturnRootBone();
	XFRAME_EX* headbone = (XFRAME_EX*)D3DXFrameFind(rootBone, "Head");
	_inverseKinematic.Initial(&headbone->CombinedTransformationMatrix);

	//���������������ͷ������
	int screenPosX, screenPosY;
	InputCtrl::GetInstance()->GetMouseLocation(&screenPosX, &screenPosY);
	D3DXVECTOR3 target;
	target.x = (screenPosX - WINDOW_WIDTH / 2.0f) / WINDOW_WIDTH*2.0f;
	target.y = (WINDOW_HEIGHT / 2.0f - screenPosY) / WINDOW_HEIGHT*2.0f + 1.5f;
	target.z = -2.0f;
	_inverseKinematic.Update(headbone, _pBody, &target);
	_pHead->Update(deltTime);
}

//���ƽ�ɫ
void Character::Render(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	//������¿ո�������л�����
	if (InputCtrl::GetInstance()->IsSpacePressed())
		_pHead->RandomHead();

	//����body
	XFRAME_EX* rootBone = (XFRAME_EX*)_pBody->ReturnRootBone();
	Render(device, rootBone, effect, model, view, projection);

	//����(eye)	
	//֮���Գ���rot��ת��������Ϊͷ��������x�����ϣ�����������ϵ��y�����ϣ�
	//������CombinedTransformationMatrix����ӹ�������ϵת��Ϊģ������ϵ���������������ת�ɷ֣�
	//����rot��ת������Ϊ�˽���ת�ɷֵ�����
	D3DXMATRIX rot;
	D3DXMatrixRotationZ(&rot, -D3DX_PI * 0.5f);

	XFRAME_EX* headbone = (XFRAME_EX*)D3DXFrameFind(rootBone, "Head");
	_headModel = headbone->CombinedTransformationMatrix;
	_headModel =  rot * _headModel * (*model);
	_pHead->RenderEye(effect, &_headModel, view, projection);
}

//�ݹ������Ƥ����
void Character::Render(IDirect3DDevice9* device, XFRAME_EX* bone, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
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

			effect->SetMatrixArray("MatPalette", boneMesh->currentBoneMaterices, numBones);
			effect->SetInt("NumBoneInfluences", boneMesh->NumInfl);

			//����head����ʱ������head(face)����ֹͣ����
			if (strcmp(bone->Name, "Face") == 0)
			{
				//����StreamSource5, ��������Ĺ��������͹���Ȩ��
				IDirect3DVertexBuffer9* headBuffer = nullptr;
				//DWORD vSize = D3DXGetFVFVertexSize(boneMesh->MeshData.pMesh->GetFVF());
				DWORD vSize = boneMesh->MeshData.pMesh->GetNumBytesPerVertex();
				boneMesh->MeshData.pMesh->GetVertexBuffer(&headBuffer);
				HRESULT hr = device->SetStreamSource(5, headBuffer, 0, vSize);

				_pHead->RenderFace(device, effect, model, view, projection);
				return;
			}

			effect->SetMatrix("Model", model);
			effect->SetMatrix("View", view);
			effect->SetMatrix("Projection", projection);

			effect->SetTechnique("TechSkin");

			for (UINT k = 0; k<boneMesh->numAttributesGroups; k++)
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
/*		else
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
		}*/
	}

	if (bone->pFrameSibling)
		Render(device, (XFRAME_EX*)bone->pFrameSibling, effect, model, view, projection);
	if (bone->pFrameFirstChild)
		Render(device, (XFRAME_EX*)bone->pFrameFirstChild, effect, model, view, projection);
}

//�ڴ�����
void Character::CleanUp()
{
	_pHead->CleanUp();
	SAFE_DELETE(_pHead);

	_pBody->CleanUp();
	SAFE_DELETE(_pBody);
}