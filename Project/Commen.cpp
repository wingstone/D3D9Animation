#include "Commen.h"

//œÚpNewName∑÷≈‰√˚◊÷
HRESULT AllocateName(LPCSTR Name, LPSTR* pNewName)
{
	if (Name != NULL)
	{
		UINT nameSize = strlen(Name);
		*pNewName = new char[nameSize + 1];
		memset(*pNewName, 0, (nameSize + 1) * sizeof(char));
		memcpy(*pNewName, Name, (nameSize + 1) * sizeof(char));
	}
	else
	{
		*pNewName = NULL;
	}

	return S_OK;
}
