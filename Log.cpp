#include"Log.h"


// Log関数
void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}