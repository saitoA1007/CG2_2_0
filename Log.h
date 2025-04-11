#pragma once
#include<iostream>
#include <Windows.h>
#include<filesystem>
#include<fstream>
#include<chrono>

// Log関数
void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}
