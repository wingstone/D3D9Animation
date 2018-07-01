#include "Head.h"

//=================Eye class=====================================//

Eye::Eye()
{
}

Eye::~Eye()
{}

//设置眼睛的位置，这里眼睛的位置是相对于Head的位置，不是相对于世界原点
void Eye::SetPos(D3DXVECTOR3 * pos)
{
	_position = *pos;
}

//更新眼睛注视目标的位置
void Eye::Update(D3DXVECTOR2 * target)
{
	D3DXMatrixTranslation(&_modelMat, _position.x, _position.y, _position.z);
	D3DXMATRIX rotMat;
	float roty = atan( (target->x - _position.x) / (-1 - _position.z) );
	float rotx = atan( (target->y - _position.y) / (-1 - _position.z));
	D3DXMatrixRotationYawPitchRoll(&rotMat, roty, rotx, 0.0f);
	_modelMat =  rotMat * _modelMat;
}

//绘制眼睛
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

//加载头部网格和纹理文件，眼睛网格纹理文件
HRESULT Head::Load(IDirect3DDevice9* device, const char* faceFile, std::vector<std::string> *texFaceList, const char* eyeFile, std::vector<std::string> *texEyeList)
{
	HRESULT hr = S_OK;

	D3DXVECTOR3 eyePos1 = D3DXVECTOR3(-0.037f, 0.125f, -0.063f);
	D3DXVECTOR3 eyePos2 = D3DXVECTOR3(0.037f, 0.125f, -0.063f);

	//创建眼睛
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

	//创建脸部
	_pFace = new MorphFace();
	hr = _pFace->Load(device, faceFile, texFaceList);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

//根据观察目标，更新眼睛的世界矩阵
void Head::Update(float deltTime)
{
	//这里target的坐标是相对于Head坐标系的，其中z坐标默认为-1
	D3DXVECTOR2 target = D3DXVECTOR2(0.0f, 0.0f);
	_pEye1->Update(&target);
	_pEye2->Update(&target);
}

//切换脸部纹理，眼睛纹理
void Head::RandomHead()
{
	_pFace->RandomFace();
	_eyeIndex = rand() % 5;
}

//绘制眼睛
void Head::RenderEye(ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	effect->SetTexture("TexDiffuse", _vecEyeTex[_eyeIndex]);
	_pEye1->Render(effect, model, view, projection);
	_pEye2->Render(effect, model, view, projection);
}

//绘制脸型，不包括眼睛
void Head::RenderFace(IDirect3DDevice9* device, ID3DXEffect* effect, D3DXMATRIX* model, D3DXMATRIX* view, D3DXMATRIX* projection)
{
	_pFace->Render(device, effect, model, view, projection);
}

//内存清理
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