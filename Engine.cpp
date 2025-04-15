#include"Engine.h"

void Engine::Initialize(const std::wstring& title = L"LE2A_05_サイトウ_アオイ", const int32_t kClientWidth, const int32_t kClientHeight) {

	Engine::WindowsAPI windowsAPI;

	windowsAPI.CreateGameWindow(L"CG2WindowClass", kClientWidth, kClientHeight);

}

void Engine::Update() {

}

void Engine::Finalize() {

}