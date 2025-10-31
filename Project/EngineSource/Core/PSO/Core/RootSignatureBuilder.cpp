#include"RootSignatureBuilder.h"
#include <cassert>
#include"LogManager.h"
#include <d3d12shader.h>

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

void RootSignatureBuilder::CreateRootSignatureFromReflection(IDxcUtils* utils,IDxcBlob* vsBlob, IDxcBlob* psBlob) {

	DxcBuffer reflectionBuffer{};
	reflectionBuffer.Encoding = DXC_CP_ACP;

	// 頂点シェーダーとピクセルシェーダーのリフレクションを取得
	IDxcBlob* shaderBlobs[2] = { vsBlob, psBlob };
	D3D12_SHADER_VISIBILITY visibilities[2] = {
		D3D12_SHADER_VISIBILITY_VERTEX,
		D3D12_SHADER_VISIBILITY_PIXEL
	};

    // 分析する
    for (uint32_t index = 0; index < 2; ++index) {
        ReflectionBoundResource(utils, reflectionBuffer, shaderBlobs[index], visibilities[index]);
    }

    // ルートシグネチャの作成
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.NumParameters = static_cast<UINT>(rootParameters_.size());
    rootSigDesc.pParameters = rootParameters_.data();
    rootSigDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());
    rootSigDesc.pStaticSamplers = staticSamplers_.data();
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob, errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            LogManager::GetInstance().Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false);
    }

    // バイナリを元に生成
    hr = device_->CreateRootSignature(0,
        signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void RootSignatureBuilder::ReflectionBoundResource(IDxcUtils* utils,DxcBuffer reflectionBuffer, IDxcBlob* shaderBlob, D3D12_SHADER_VISIBILITY visibility) {
    reflectionBuffer.Ptr = shaderBlob->GetBufferPointer();
    reflectionBuffer.Size = shaderBlob->GetBufferSize();

    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
    HRESULT hr = utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection));
    if (FAILED(hr)) {
        assert(0);
    }

    D3D12_SHADER_DESC shaderDesc{};
    reflection->GetDesc(&shaderDesc);

    // バインドされたリソースを解析
    for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc{};
        reflection->GetResourceBindingDesc(i, &bindDesc);

        D3D12_ROOT_PARAMETER param{};
        param.ShaderVisibility = visibility;

        switch (bindDesc.Type) {

            // 定数バッファ
        case D3D_SIT_CBUFFER: {
            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            param.Descriptor.ShaderRegister = bindDesc.BindPoint;
            param.Descriptor.RegisterSpace = bindDesc.Space;
            rootParameters_.push_back(param);
            break;
        }


            // SRV
        case D3D_SIT_TEXTURE: {
            D3D12_DESCRIPTOR_RANGE range{};
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = bindDesc.BindCount;
            range.BaseShaderRegister = bindDesc.BindPoint;
            range.RegisterSpace = bindDesc.Space;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            descriptorRanges_.push_back(range);

            param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param.DescriptorTable.NumDescriptorRanges = 1;
            param.DescriptorTable.pDescriptorRanges = &descriptorRanges_.back();
            rootParameters_.push_back(param);
            break;
        }


        // サンプラー
        case D3D_SIT_SAMPLER: {
            D3D12_STATIC_SAMPLER_DESC samplerDesc{};
            samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
            samplerDesc.ShaderRegister = bindDesc.BindPoint;
            samplerDesc.RegisterSpace = bindDesc.Space;
            samplerDesc.ShaderVisibility = visibility;
            staticSamplers_.push_back(samplerDesc);
            break;
        }
        }
    }
}