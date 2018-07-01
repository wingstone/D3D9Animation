#include "Debug.h"

Debug::Debug()
{}

Debug::~Debug()
{}

std::fstream Debug::_fileStream;

//���ļ���
bool Debug::OpenFile(char* fileStr)
{
	_fileStream.open(fileStr, std::fstream::out | std::fstream::trunc);
	if (_fileStream.fail())
		return false;

	return true;
}

//���ļ������ַ���
void Debug::OutputMessage(char* str)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	_fileStream << st.wHour << ": " << st.wMinute << ": " << st.wSecond << "    ";
	_fileStream << str << std::endl;
}

//�ر��ļ���
void Debug::CloseFile()
{
	_fileStream.close();
}