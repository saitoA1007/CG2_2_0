#include"PSOManager.h"
#include "LogManager.h"
#include <cassert>

using namespace GameEngine;

void PSOManager::Initialize(ID3D12Device* device, DXC* dxc) {
	LogManager::GetInstance().Log("Initialize PSOManager Start");
	device_ = device;
	dxc_ = dxc;
	// ラスタライザの全パターン生成
	rasterizerBuiler_.Initialize();
	// ブレンドモードの全パターン生成
	blendBuilder_.Initialize();
	LogManager::GetInstance().Log("Initialize PSOManager End");
}

void PSOManager::RegisterPSO(const std::string& name) {

	

	LogManager::GetInstance().Log("PSO registerd name : " + name);
}

void PSOManager::LoadFromJson(const std::string& filePath) {
	LogManager::GetInstance().Log("PSO registerd name : " + filePath);
}

void PSOManager::DefaultLoadPSO() {

}

void PSOManager::CreatePSO(const std::string& name) {
	LogManager::GetInstance().Log("PSO registerd name : " + name);
}

void PSOManager::CreatePSO(const std::string& psoName, PSOData psoData) {
	LogManager::GetInstance().Log("PSO registerd name : " + psoName);
	(void)psoData;
}