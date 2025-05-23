#include"WorldTransforms.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Common/DescriptorHandle.h"
using namespace GameEngine;

DirectXCommon* WorldTransforms::dxCommon_ = nullptr;
uint32_t WorldTransforms::StaticSrvIndex_ = 0;

void WorldTransforms::StaticInitialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

void WorldTransforms::Initialize(const uint32_t& kNumInstance) {

	TransformData transformData;
	for (uint32_t i = 0; i < kNumInstance; ++i) {
		transformData.transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
		transformData.worldMatrix = MakeAffineMatrix(transformData.transform.scale, transformData.transform.rotate, transformData.transform.translate);
		transformData.color = { 1.0f,1.0f,1.0f,1.0f };
		transformDatas_.push_back(transformData);
	}

	// 頂点数を設定
	numInstance = kNumInstance;

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParticleForGPU) * numInstance);
	// 書き込むためのアドレスを取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < numInstance; ++index) {
		instancingData_[index].WVP = MakeIdentity4x4();
		instancingData_[index].World = MakeIdentity4x4();
		instancingData_[index].color = { 1.0f,1.0f,1.0f,1.0f };
	}

	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = numInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	instancingSrvHandleCPU_ = GetCPUDescriptorHandle(dxCommon_->GetSRVHeap(), dxCommon_->GetSRVDescriptorSize(), StaticSrvIndex_ + 130);
	instancingSrvHandleGPU_ = GetGPUDescriptorHandle(dxCommon_->GetSRVHeap(), dxCommon_->GetSRVDescriptorSize(), StaticSrvIndex_ + 130);
	dxCommon_->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU_);

	// srvを更新
	StaticSrvIndex_++;
}

void WorldTransforms::UpdateTransformMatrix(const uint32_t& numInstance) {
	// 数によって更新を変える
	for (uint32_t i = 0; i < numInstance; ++i) {
		transformDatas_[i].worldMatrix = MakeAffineMatrix(transformDatas_[i].transform.scale, transformDatas_[i].transform.rotate, transformDatas_[i].transform.translate);
	}
}

void WorldTransforms::SetWVPMatrix(const uint32_t& numInstance,const Matrix4x4& VPMatrix) {
	// 数によって更新を変える
	for (uint32_t i = 0; i < numInstance; ++i) {
		instancingData_[i].WVP = Multiply(transformDatas_[i].worldMatrix, VPMatrix);
		instancingData_[i].World = transformDatas_[i].worldMatrix;
		instancingData_[i].color = transformDatas_[i].color;
	}
}