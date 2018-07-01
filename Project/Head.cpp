#include "Head.h"

//=================Eye class=====================================//

Eye::Eye()
{
}

Eye::~Eye()
{}

//�����۾���λ�ã������۾���λ���������Head��λ�ã��������������ԭ��
void Eye::SetPos(D3DXVECTOR3 * pos)
{
	_position = *pos;
}

//�����۾�ע��Ŀ���λ��
void Eye::Update(D3DXVECTOR2 * target)
{
	D3DXMatrixTranslation(&_modelMat, _position.x, _position.y, _position.z);
	D3DXMATRIX rotMat;
	float roty = atan( (target->x - _position.x) / (-1 - _position.z) );
	float rotx = atan( (target->y - _position.y) / (-1 - _position.z));
	D3DXMatrixRotationYawPitchRoll(&rotMat, roty, rotx, 0.0f);
	_modelMat =  rotMat * _modelMat;
}

//�����۾�
void Eye::Render(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	_modelMat = _modelMat * (*model);

	StaticMesh::Render(effect, &_modelMat, view, projection);
}

//=================Head class=====================================//

Head::Head()
{
	_pEye1 = nullptr;
	_pEye2 = nullptr;
	_eyeIndex = 0;
	_pFace = nullptr;
}

Head::~Head()
{
}

//����ͷ������������ļ����۾����������ļ�
HRESULT Head::Load(IDirect3DDevice9* device, const char* faceFile, std::vector<std::string> *texFaceList, const char* eyeFile, std::vector<std::string> *texEyeList)
{
	HRESULT hr = S_OK;

	D3DXVECTOR3 eyePos1 = D3DXVECTOR3(-0.037f, 0.125f, -0.063f);
	D3DXVECTOR3 eyePos2 = D3DXVECTOR3(0.037f, 0.125f, -0.063f);

	//�����۾�
	_pEye1 = new Eye();
	hr = _pEye1->Load(device, eyeFile);
	if(FAILED(hr))
		return hr;
	_pEye1->SetPos(&eyePos1);

	_pEye2 = new Eye();
	hr = _pEye2->Load(device, eyeFile);
	if (FAILED(hr))
		return hr;
	_pEye2->SetPos(&eyePos2);

	//load texture
	for (size_t i = 0; i < texEyeList->size(); i++)
	{
		IDirect3DTexture9* pTex;
		hr = D3DXCreateTextureFromFile(device, (*texEyeList)[i].c_str(), &pTex);
		_vecEyeTex.push_back(pTex);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	//��������
	_pFace = new MorphFace();
	hr = _pFace->Load(device, faceFile, texFaceList);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

//���ݹ۲�Ŀ�꣬�����۾����������
void Head::Update(float deltTime)
{
	//����target�������������Head����ϵ�ģ�����z����Ĭ��Ϊ-1
	D3DXVECTOR2 target = D3DXVECTOR2(0.0f, 0.0f);
	_pEye1->Update(&target);
	_pEye2->Update(&target);
}

//�л����������۾�����
void Head::RandomHead()
{
	_pFace->RandomFace();
	_eyeIndex = rand() % 5;
}

//�����۾�
void Head::RenderEye(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	effect->SetTexture("TexDiffuse", _vecEyeTex[_eyeIndex]);
	_pEye1->Render(effect, model, view, projection);
	_pEye2->Render(effect, model, view, projection);
}

//�������ͣ��������۾�
void Head::RenderFace(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	_pFace->Render(device, effect, model, view, projection);
}

//�ڴ�����
void Head::CleanUp()
{
	if (_pEye1)
	{
		_pEye1->CleanUp();
		SAFE_DELETE(_pEye1);
	}

	if (_pEye2)
	{
		_pEye2->CleanUp();
		SAFE_DELETE(_pEye2);
	}

	if (_pFace)
	{
		_pFace->CleanUp();
		SAFE_DELETE(_pFace);
	}
}