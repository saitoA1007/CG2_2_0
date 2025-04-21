#pragma once
#include<iostream>
#include<fstream>
#include <Windows.h>
#include<filesystem>
#include<chrono>

// Log関数
inline void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}