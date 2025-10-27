#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<vector>
#include<string>
#include <wrl.h>

namespace GameEngine {

	class RootSignatureBuilder {
	public:

		void Initialize(ID3D12Device* device);

		void AddCBVParameter(uint32_t shaderRegister, D3D12_SHADER_VISIBILITY visibility);
		void AddSRVDescriptorTable(uint32_t shaderRegister, uint32_t arryNum, D3D12_SHADER_VISIBILITY visibility);

		void CreateRootSignature();

	private:

		ID3D12Device* device_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters_;

		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	};
}