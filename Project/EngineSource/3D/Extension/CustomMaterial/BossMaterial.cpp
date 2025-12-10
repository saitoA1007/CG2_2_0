#include"BossMaterial.h"
#include"MyMath.h"
#include"CreateBufferResource.h"
ID3D12Device* BossMaterial::device_ = nullptr;

BossMaterial::~BossMaterial() {
	// マッピングを解除する
	if (materialData_) {
		materialResource_->Unmap(0, nullptr);
		materialData_ = nullptr;
	}
}

void BossMaterial::StaticInitialize(ID3D12Device* device) {
	device_ = device;
}

void BossMaterial::Initialize() {
	// マテリアルリソースを作成
	// マテリアル用のリソースを作る。color1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(device_, sizeof(MaterialData));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 白色に設定
	materialData_->color = { 1.0f,1.0f,1.0f,1.0f };
	// Lightingするのでtureに設定する
	materialData_->enableLighting = false;
	// UVTransform行列を初期化
	materialData_->uvTransform = MakeIdentity4x4();
	// specularの色を設定
	materialData_->specularColor = { 1.0f,1.0f,1.0f };
	// 輝度を設定
	materialData_->shininess = 250.0f;
	// テクスチャデータ
	materialData_->textureHandle = 0;
	materialData_->noiseTexture = 0;
	materialData_->time = 0.0f;
}