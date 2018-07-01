#ifndef DEBUG_H
#define DEBUG_H

#include <fstream>
#include <Windows.h>

//����ģʽ�������ļ������־
class Debug
{
public:
	static bool OpenFile(char* fileStr);
	static void OutputMessage(char* messageStr);
	static void CloseFile();

private:
	static std::fstream _fileStream;

	Debug();
	~Debug();
};


#endif