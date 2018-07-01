#ifndef INVERSEKINEMATIC
#define INVERSEKINEMATIC

#include <d3d9.h>
#include <d3dx9.h>
#include<Windows.h>
#include<math.h>

#include"Commen.h"
#include"Debug.h"
#include "SkinMesh.h"

//用于加载静态网格
class InverseKinematic_Head
{
public:
	InverseKinematic_Head();
	InverseKinematic_Head(const InverseKinematic_Head& other);
	~InverseKinematic_Head();

	void Initial(D3DXMATRIX * headCombineMatrix);
	void Update(XFRAME_EX * headFrame, SkinMesh *skinMesh, D3DXVECTOR3 *targetPos);

private:
	D3DXVECTOR3 _headForward;
};

#endif // !STATICMESH
