#ifndef DEBUG_H
#define DEBUG_H

#include <fstream>
#include <Windows.h>

//单例模式，用于文件输出日志
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