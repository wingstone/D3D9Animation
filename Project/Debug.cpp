#include "Debug.h"

Debug::Debug()
{}

Debug::~Debug()
{}

std::fstream Debug::_fileStream;

//打开文件流
bool Debug::OpenFile(char* fileStr)
{
	_fileStream.open(fileStr, std::fstream::out | std::fstream::trunc);
	if (_fileStream.fail())
		return false;

	return true;
}

//向文件输入字符串
void Debug::OutputMessage(char* str)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	_fileStream << st.wHour << ": " << st.wMinute << ": " << st.wSecond << "    ";
	_fileStream << str << std::endl;
}

//关闭文件流
void Debug::CloseFile()
{
	_fileStream.close();
}