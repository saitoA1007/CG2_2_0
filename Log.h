#pragma once
#include<iostream>
#include <Windows.h>

// Log関数
void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}
