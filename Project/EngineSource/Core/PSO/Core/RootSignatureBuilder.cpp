#include"RootSignatureBuilder.h"
#include <cassert>
#include"LogManager.h"

using namespace GameEngine;

void RootSignatureBuilder::Initialize(ID3D12Device* device) {
	device_ = device;
	descriptionRootSignature_.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

void RootSignatureBuilder::AddCBVParameter(uint32_t shaderRegister, D3D12_SHADER_VISIBILITY visibility) {
	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param.ShaderVisibility = visibility;
	param.Descriptor.ShaderRegister = shaderRegister;
	rootParameters_.push_back(param);
}

void RootSignatureBuilder::AddSRVDescriptorTable(uint32_t shaderRegister, uint32_t arryNum, D3D12_SHADER_VISIBILITY visibility) {
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = shaderRegister; // 始まる番号
	descriptorRange[0].NumDescriptors = arryNum; // 数
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを自動計算

	D3D12_ROOT_PARAMETER param{};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	param.ShaderVisibility = visibility; // 使うシェーダー
	param.DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	param.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters_.push_back(param);
}

void RootSignatureBuilder::CreateRootSignature() {
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		LogManager::GetInstance().Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	hr = device_->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}