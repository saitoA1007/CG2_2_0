#pragma once
#include <d3d12.h>
#include<dxcapi.h>
#include<vector>
#include<string>
#include <wrl.h>

namespace GameEngine {

	// パラメータタイプを設定した
	enum class ParameterType {
		CBV,
		SRV
	};

	class RootSignatureBuilder {
	public:

		void Initialize(ID3D12Device* device);

		void AddCBVParameter(uint32_t shaderRegister, D3D12_SHADER_VISIBILITY visibility);
		void AddSRVDescriptorTable(uint32_t shaderRegister, uint32_t arryNum, D3D12_SHADER_VISIBILITY visibility);

		void AddSampler(uint32_t shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE texAddress, D3D12_SHADER_VISIBILITY visibility);

		void CreateRootSignature();

		/// <summary>
		/// shaderReflectionを使用して自動生成
		/// </summary>
		/// <param name="vsBlob"></param>
		/// <param name="psBlob"></param>
		void CreateRootSignatureFromReflection(IDxcUtils* utils,IDxcBlob* vsBlob, IDxcBlob* psBlob);

		ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

		std::vector<ParameterType> GetParameterTypes() const { return parameterTypes_; }

	private:

		ID3D12Device* device_ = nullptr;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

		std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
		std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges_;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;
		
		// パラメータのタイプ
		std::vector<ParameterType> parameterTypes_;

	private:

		/// <summary>
		/// バインドされたリソースを解析する
		/// </summary>
		void ReflectionBoundResource(IDxcUtils* utils, DxcBuffer reflectionBuffer, IDxcBlob* shaderBlob,D3D12_SHADER_VISIBILITY visibility);

	};
}