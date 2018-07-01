#include "InverseKinematic.h"

InverseKinematic_Head::InverseKinematic_Head()
{}

InverseKinematic_Head::~InverseKinematic_Head()
{}

//根据头部组合矩阵，将头部forward向量转换至head坐标系
void InverseKinematic_Head::Initial(D3DXMATRIX * headCombineMatrix)
{
	D3DXMATRIX headMat = *headCombineMatrix;
	headMat._41 = headMat._42 = headMat._43 = 0;		//去除平移成分，后面的矩阵变换使用对点进行变换
	headMat._44 = 1;																//不去除平移成分也可以，后面的矩阵变换使用对向量进行变换就行

	D3DXMATRIX toHeadSpace;
	D3DXMatrixInverse(&toHeadSpace, nullptr, &headMat);		//中间的参数可以返回原矩阵行列式的值

	D3DXVECTOR3 headforward = D3DXVECTOR3(0, 0, -1);
	D3DXVECTOR4 tmp;
	D3DXVec3Transform(&tmp, &headforward, &toHeadSpace);

	_headForward = D3DXVECTOR3(tmp.x, tmp.y, tmp.z);
}

//根据输入坐标(位于世界坐标系)，更新头部骨骼矩阵
void InverseKinematic_Head::Update(XFRAME_EX * headFrame, SkinMesh *skinMesh, D3DXVECTOR3 *targetPos)
{
	D3DXMATRIX headMat = headFrame->CombinedTransformationMatrix;
	D3DXMATRIX inverseHeadMat;
	D3DXMatrixInverse(&inverseHeadMat, nullptr, &headMat);

	D3DXVECTOR4 tmp;
	D3DXVec3Transform(&tmp, targetPos, &inverseHeadMat);
	D3DXVECTOR3 headSpaceTarget = D3DXVECTOR3(tmp.x, tmp.y, tmp.z);
	D3DXVec3Normalize(&headSpaceTarget, &headSpaceTarget);

	D3DXVECTOR3 axis;
	D3DXVec3Cross(&axis, &_headForward, &headSpaceTarget);
	D3DXVec3Normalize(&axis, &axis);

	float angle = D3DXVec3Dot(&_headForward, &headSpaceTarget);
	angle = acos(angle);
	angle = min(angle, D3DX_PI / 3);

	D3DXMATRIX rotMat;
	D3DXMatrixRotationAxis(&rotMat, &axis, angle);

	headFrame->CombinedTransformationMatrix = rotMat * headFrame->CombinedTransformationMatrix;
	skinMesh->UpdataFrame((XFRAME_EX *)headFrame->pFrameFirstChild, &headFrame->CombinedTransformationMatrix);
}