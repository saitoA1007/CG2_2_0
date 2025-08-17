#include"WorldTransform.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Common/CreateBufferResource.h"
using namespace GameEngine;

ID3D12Device* WorldTransform::device_ = nullptr;

void WorldTransform::StaticInitialize(ID3D12Device* device) {
	device_ = device;
}

void WorldTransform::Initialize(const Transform& transform) {
	transform_ = transform;
	//worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	worldMatrix_ = MakeWorldMatrixFromEulerRotation(transform_.translate, transform_.rotate, transform_.scale);

	// トランスフォーメーション行列リソースを作成
	// TransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(device_, sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->worldInverseTranspose = MakeIdentity4x4();
}

void WorldTransform::UpdateTransformMatrix() {
	//worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	worldMatrix_ = MakeWorldMatrixFromEulerRotation(transform_.translate, transform_.rotate, transform_.scale);
	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->worldInverseTranspose = InverseTranspose(worldMatrix_);
}

void WorldTransform::SetWVPMatrix(const Matrix4x4& localMatrix,const Matrix4x4& VPMatrix) {
	transformationMatrixData_->WVP = Multiply(localMatrix ,Multiply(worldMatrix_, VPMatrix));
	transformationMatrixData_->World = Multiply(localMatrix,worldMatrix_);
	transformationMatrixData_->worldInverseTranspose = InverseTranspose(worldMatrix_);
}

void WorldTransform::SetWVPMatrix(const Matrix4x4& VPMatrix) {
	transformationMatrixData_->WVP = Multiply(worldMatrix_, VPMatrix);
	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->worldInverseTranspose = InverseTranspose(worldMatrix_);
}