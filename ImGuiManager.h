#pragma once
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include <Windows.h>

class ImGuiManager {
public:

	void Initialize(HWND hwnd,);

	void Update();

	void Finalize()

private:


};


