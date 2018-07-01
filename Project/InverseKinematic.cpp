#include "InverseKinematic.h"

InverseKinematic_Head::InverseKinematic_Head()
{}

InverseKinematic_Head::~InverseKinematic_Head()
{}

//����ͷ����Ͼ��󣬽�ͷ��forward����ת����head����ϵ
void InverseKinematic_Head::Initial(D3DXMATRIX * headCombineMatrix)
{
	D3DXMATRIX headMat = *headCombineMatrix;
	headMat._41 = headMat._42 = headMat._43 = 0;		//ȥ��ƽ�Ƴɷ֣�����ľ���任ʹ�öԵ���б任
	headMat._44 = 1;																//��ȥ��ƽ�Ƴɷ�Ҳ���ԣ�����ľ���任ʹ�ö��������б任����

	D3DXMATRIX toHeadSpace;
	D3DXMatrixInverse(&toHeadSpace, nullptr, &headMat);		//�м�Ĳ������Է���ԭ��������ʽ��ֵ

	D3DXVECTOR3 headforward = D3DXVECTOR3(0, 0, -1);
	D3DXVECTOR4 tmp;
	D3DXVec3Transform(&tmp, &headforward, &toHeadSpace);

	_headForward = D3DXVECTOR3(tmp.x, tmp.y, tmp.z);
}

//������������(λ����������ϵ)������ͷ����������
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