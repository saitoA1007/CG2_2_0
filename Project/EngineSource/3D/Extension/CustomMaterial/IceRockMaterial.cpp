#include"IceRockMaterial.h"
#include"MyMath.h"
#include"CreateBufferResource.h"
ID3D12Device* IceRockMaterial::device_ = nullptr;

IceRockMaterial::~IceRockMaterial() {
	// マッピングを解除する
	if (materialData_) {
		materialResource_->Unmap(0, nullptr);
		materialData_ = nullptr;
	}
}

void IceRockMaterial::StaticInitialize(ID3D12Device* device) {
	device_ = device;
}

void IceRockMaterial::Initialize() {
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
	materialData_->specularColor = {1.0f,1.0f,1.0f};
	// 輝度を設定
	materialData_->shininess = 250.0f;
	// テクスチャデータ
	materialData_->textureHandle = 0;
	// リムライト
	materialData_->rimColor = { 0.0f,0.296f,0.672f };
	materialData_->rimPower = 3.0f;
	materialData_->rimIntensity = 0.3f;
}