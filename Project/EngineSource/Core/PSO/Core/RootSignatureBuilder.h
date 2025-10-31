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

		/// <summary>
		/// shaderReflectionを使用して自動生成
		/// </summary>
		/// <param name="vsBlob"></param>
		/// <param name="psBlob"></param>
		void CreateRootSignatureFromReflection(IDxcUtils* utils,IDxcBlob* vsBlob, IDxcBlob* psBlob);

		ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

	private:

		ID3D12Device* device_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
		std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges_;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;

		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};

	private:

		/// <summary>
		/// バインドされたリソースを解析する
		/// </summary>
		void ReflectionBoundResource(IDxcUtils* utils, DxcBuffer reflectionBuffer, IDxcBlob* shaderBlob,D3D12_SHADER_VISIBILITY visibility);

	};
}