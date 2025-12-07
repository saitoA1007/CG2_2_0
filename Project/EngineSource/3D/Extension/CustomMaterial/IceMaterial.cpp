#include"IceMaterial.h"
#include"MyMath.h"
#include"CreateBufferResource.h"
ID3D12Device* IceMaterial::device_ = nullptr;

IceMaterial::~IceMaterial() {
	// マッピングを解除する
	if (materialData_) {
		materialResource_->Unmap(0, nullptr);
		materialData_ = nullptr;
	}
}

void IceMaterial::StaticInitialize(ID3D12Device* device) {
	device_ = device;
}

void IceMaterial::Initialize() {
	// マテリアルリソースを作成
	// マテリアル用のリソースを作る。color1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(device_, sizeof(MaterialData));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 白色に設定
	materialData_->baseColor = { 1.0f,1.0f,1.0f,1.0f };
	materialData_->color = {1.0f,1.0f,1.0f,1.0f};
	// UVTransform行列を初期化
	materialData_->uvTransform = MakeIdentity4x4();
	// specularの色を設定
	materialData_->specularColor = {0.6f,0.89f,0.96f};
	// 輝度を設定
	materialData_->shininess = 100.0f;
	// テクスチャデータ
	materialData_->textureHandle = 0;
	materialData_->normalTextureHandle = 0;
	materialData_->baseTextureHandle = 0;
	// 時間
	materialData_->time = 3.0f;
	// リムライト
	materialData_->rimColor = { 0.0f,0.296f,0.672f };
	materialData_->rimIntensity = 0.3f;
	materialData_->rimPower = 3.0f;
	// 平面
	materialData_->planeSpecularColor = { 1.0f,1.0f,1.0f,0.0f };
	materialData_->planeSpecularPower = 250.0f;
}