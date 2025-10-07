#include"Animation.h"
#include <algorithm>

#include"CreateBufferResource.h"
#include"DescriptorHandle.h"
#include"MyMath.h"
#include<cassert>
#include"EasingManager.h"
using namespace GameEngine;

ID3D12Device* Animation::device_ = nullptr;
ID3D12DescriptorHeap* Animation::srvHeap_ = nullptr;
uint32_t Animation::descriptorSizeSRV_ = 0;

void Animation::StaticInitialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, const uint32_t& descriptorSizeSRV) {
	device_ = device;
	srvHeap_ = srvHeap;
	descriptorSizeSRV_ = descriptorSizeSRV;
}

SkinCluster Animation::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData) {

	assert(!skeleton.joints.empty() && "Skeleton joints are empty!");
	assert(!modelData.meshes.empty() && "Model has no meshes!");
	assert(!modelData.meshes[0].vertices.empty() && "Model mesh[0] has no vertices!");

	SkinCluster skinCluster;

	// palette用のResourceを確保
	skinCluster.paletteResource = CreateBufferResource(device_, sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0,nullptr,reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette,skeleton.joints.size() }; // spanを使ってアクセスするようにする
	skinCluster.paletteSrvHandle.first = GetCPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, 180);
	skinCluster.paletteSrvHandle.second = GetGPUDescriptorHandle(srvHeap_, descriptorSizeSRV_, 180);

	// palette用のsrvを作成。StructuredBufferでアクセス出来るようにする。
	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
	paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	paletteSrvDesc.Buffer.FirstElement = 0;
	paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	paletteSrvDesc.Buffer.NumElements = static_cast<UINT>(skeleton.joints.size());
	paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
	device_->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);

	// influence用のResourceを確保。頂点ごとにinfluence情報を追加出来るようにする
	skinCluster.influenceResource = CreateBufferResource(device_, sizeof(VertexInfluence) * modelData.meshes[0].vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.meshes[0].vertices.size()); // 0埋め。weightを0にしておく
	skinCluster.mappedInfluence = { mappedInfluence,modelData.meshes[0].vertices.size() };

	// influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexInfluence) * modelData.meshes[0].vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// inverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), MakeIdentity4x4);

	// ModelのSkinClusterの情報を解析
	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton.jointMap.find(jointWeight.first); // jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
		if (it == skeleton.jointMap.end()) { // 存在しない場合は次に回す
			continue;
		}
		// (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex]; // 該当のvertexIndexのinfluence情報を参照しておく
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) { // 空いているところに入れる
				if (currentInfluence.weights[index] == 0.0f) { // weight==0が空いている状態なので、その場所にweightとjointのindexを代入
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}
	}

	return skinCluster;
}

void Animation::Update(SkinCluster& skinCluster, Skeleton& skeleton, const AnimationData& animation, float animationTime) {

	// アニメーションの更新をおこない、骨ごとのLocal情報を更新する
	ApplyAnimation(skeleton, animation, animationTime);

	// 現在の骨ごとのLocal情報を基にSkeletonSpaceの情報を更新する
	SkeletonUpdate(skeleton);

	// SkeletonSpaceの情報を基に、SkinClusterのMatrixPaletteを更新する
	SkinClusterUpdate(skinCluster, skeleton);
}

void Animation::SkinClusterUpdate(SkinCluster& skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix = skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = InverseTranspose(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix);
	}
}

Vector3 Animation::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {

	assert(!keyframes.empty()); // キーがないものはエラーを返す

	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = ((time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time));
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	// 一番後の時刻より後ろなので最後の値を返す
	return (*keyframes.rbegin()).value;
}

Quaternion Animation::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {

	assert(!keyframes.empty()); // キーがないものはエラーを返す

	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;

		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = ((time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time));
			return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	// 一番後の時刻より後ろなので最後の値を返す
	return (*keyframes.rbegin()).value;
}

void Animation::ApplyAnimation(Skeleton& skeleton, const AnimationData& animation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		// 対象のJointのAnimationがあれば、値の適応を行う。
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
			joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);
		}
	}
}

void Animation::SkeletonUpdate(Skeleton& skeleton) {
	// すべてのJointを更新。
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}