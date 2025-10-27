#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include <wrl.h>
#include<unordered_map>
#include<string>

#include"DXC.h"

namespace GameEngine {

	class PSOManager {
	public:

		// 初期化処理
		void Initialize(ID3D12Device* device, DXC* dxc);

		void RegisterPSO(const std::string& name);

		void LoadFromJson(const std::string& filePath);

		void DefaultLoadPSO();

	private:

		ID3D12Device* device_ = nullptr;
		DXC* dxc_ = nullptr;

		// PSOのリスト
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoList_;

		// ルートシグネチャリスト
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatureList_;


	private:

		void CreatePSO(const std::string& name);

	};
}

