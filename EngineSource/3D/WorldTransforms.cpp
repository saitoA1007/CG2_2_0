#include"WorldTransforms.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Common/DescriptorHandle.h"
using namespace GameEngine;

DirectXCommon* WorldTransforms::dxCommon_ = nullptr;
uint32_t WorldTransforms::StaticSrvIndex_ = 0;
std::queue<uint32_t> WorldTransforms::availableIndices_;
std::unordered_set<uint32_t> WorldTransforms::usedIndices_;
uint32_t WorldTransforms::nextNewIndex_ = 0;
const uint32_t WorldTransforms::kMaxSrvIndex_ = static_cast<uint32_t>(ResourceCount::kMaxModelCount) - static_cast<uint32_t>(ResourceCount::kStartModelCount);

WorldTransforms::~WorldTransforms() {

	// リソースの解放
	if (instancingResource_) {
		if (instancingData_) {
			instancingResource_->Unmap(0, nullptr);
			instancingData_ = nullptr;
		}
		instancingResource_.Reset();
	}

	// SRVインデックスを解放
	ReleaseSrvIndex(srvIndex_);
	transformDatas_.clear();
}

void WorldTransforms::StaticInitialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon; 

	nextNewIndex_ = 0;
}

void WorldTransforms::Initialize(const uint32_t& kNumInstance, const Transform& transform) {

	TransformData transformData;
	for (uint32_t i = 0; i < kNumInstance; ++i) {
		transformData.transform = transform;
		transformData.worldMatrix = MakeAffineMatrix(transformData.transform.scale, transformData.transform.rotate, transformData.transform.translate);
		transformData.color = { 1.0f,1.0f,1.0f,1.0f };
		transformDatas_.push_back(transformData);
	}

	// 頂点数を設定
	numInstance_ = kNumInstance;

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParticleForGPU) * numInstance_);
	// 書き込むためのアドレスを取得
	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < numInstance_; ++index) {
		instancingData_[index].WVP = MakeIdentity4x4();
		instancingData_[index].World = MakeIdentity4x4();
		instancingData_[index].color = { 1.0f,1.0f,1.0f,1.0f };
	}

	// SRVのインデックスを取得
	srvIndex_ = AddSrvIndex();
	// 指定のリソースの総数を超えたらエラーを発生
	assert(static_cast<uint32_t>(ResourceCount::kMaxModelCount) > srvIndex_ + static_cast<uint32_t>(ResourceCount::kStartModelCount));
	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = numInstance_;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	instancingSrvHandleCPU_ = GetCPUDescriptorHandle(dxCommon_->GetSRVHeap(), dxCommon_->GetSRVDescriptorSize(), srvIndex_ + static_cast<uint32_t>(ResourceCount::kStartModelCount));
	instancingSrvHandleGPU_ = GetGPUDescriptorHandle(dxCommon_->GetSRVHeap(), dxCommon_->GetSRVDescriptorSize(), srvIndex_ + static_cast<uint32_t>(ResourceCount::kStartModelCount));
	dxCommon_->GetDevice()->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU_);
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

void WorldTransforms::SetWVPMatrix(const uint32_t& numInstance, const Matrix4x4& localMatrix, const Matrix4x4& VPMatrix) {

	// 数によって更新を変える
	for (uint32_t i = 0; i < numInstance; ++i) {
		instancingData_[i].WVP = Multiply(localMatrix, Multiply(transformDatas_[i].worldMatrix, VPMatrix));
		instancingData_[i].World = transformDatas_[i].worldMatrix;
		instancingData_[i].color = transformDatas_[i].color;
	}
}

uint32_t WorldTransforms::AddSrvIndex() {

	uint32_t index = 0;

	// 利用可能であれば使用する
	if (!availableIndices_.empty()) {
		index = availableIndices_.front();
		availableIndices_.pop();
	} else if (nextNewIndex_ < kMaxSrvIndex_) {
		index = nextNewIndex_++;
	} else {
		assert(0);
		return kMaxSrvIndex_;
	}

	// 使用中のインデックスをセット
	usedIndices_.insert(index);
	return index;
}

void WorldTransforms::ReleaseSrvIndex(const uint32_t& index) {

	// 使用中のインデックスを削除
	std::unordered_set<uint32_t>::iterator usedIndex = usedIndices_.find(index);
	// 要素が見つかれば削除
	if (usedIndex != usedIndices_.end()) {
		usedIndices_.erase(usedIndex);
		// 再利用を可能にする
		availableIndices_.push(index);
	}
}