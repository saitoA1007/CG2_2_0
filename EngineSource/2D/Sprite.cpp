#include"Sprite.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Core/TextureManager.h"
using namespace GameEngine;

ID3D12Device* Sprite::device_ = nullptr;
ID3D12GraphicsCommandList* Sprite::commandList_ = nullptr;
Matrix4x4 Sprite::orthoMatrix_;
TextureManager* Sprite::textureManager_ = nullptr;
SpritePSO* Sprite::spritePSO_ = nullptr;

void Sprite::StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager, SpritePSO* spritePSO, int32_t width, int32_t height) {
	device_ = device;
	commandList_ = commandList;
	orthoMatrix_ = Multiply(MakeIdentity4x4(), MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 100.0f));
	textureManager_ = textureManager;
	spritePSO_ = spritePSO;
}

void Sprite::PreDraw(BlendMode blendMode) {
	commandList_->SetGraphicsRootSignature(spritePSO_->GetRootSignature());  // RootSignatureを設定。
	commandList_->SetPipelineState(spritePSO_->GetPipelineState(blendMode)); // spritePSOを設定
}

Sprite* Sprite::Create(const Vector2& position,const Vector2& size,const Vector2& anchorPoint,const Vector4& color) {

	Sprite* sprite = new Sprite();

	// 座標と大きさを取得
	sprite->position_ = position;
	sprite->size_ = size;
	sprite->anchorPoint_ = anchorPoint;
	// 座標を元にワールド行列の生成
	sprite->worldMatrix_ = MakeTranslateMatrix({ position.x,position.y,0.0f });

	// メッシュを作成
	sprite->CreateMesh();

	// マテリアルを作成
	sprite->CreateConstBufferData(color);

	return sprite;
}

void Sprite::Draw(const uint32_t& textureHandle) {
	// Spriteの描画。
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList_->IASetIndexBuffer(&indexBufferView_);// IBVを設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// CBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, constBufferResource_->GetGPUVirtualAddress());
	if (textureHandle != 1024) {
		// SpriteがuvCheckerを描画するようにする
		commandList_->SetGraphicsRootDescriptorTable(1, textureManager_->GetTextureSrvHandlesGPU(textureHandle));
	}
	// 描画
	commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::SetPosition(const Vector2& position) {
	position_ = position;
	// 座標を元にワールド行列の生成
	worldMatrix_ = MakeTranslateMatrix({ position.x,position.y,0.0f });
	// 座標を適用 
	constBufferData_->WVP = Multiply(worldMatrix_, orthoMatrix_);
}

void Sprite::SetSize(const Vector2& size) {
	size_ = size;
	// 画像のサイズを決める	
	float left = -anchorPoint_.x * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = -anchorPoint_.y * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;

	// 頂点インデックス
	vertexData_[0].position = { left,bottom,0.0f,1.0f }; // 左下
	vertexData_[1].position = { left,top,0.0f,1.0f }; // 左上
	vertexData_[2].position = { right,bottom,0.0f,1.0f }; // 右下
	vertexData_[3].position = { right,top,0.0f,1.0f }; // 左上
}

void Sprite::SetColor(const Vector4& color) {
	// 色の設定
	constBufferData_->color = color;
}

void Sprite::SetUvMatrix(const Transform& transform) {
	// uv行列の設定
	constBufferData_->uvTransform = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
}

void Sprite::CreateMesh() {
	// Sprite用の頂点リソースを作る
	vertexResource_ = CreateBufferResource(device_, sizeof(VertexPosUv) * 4);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexPosUv) * 4;
	// 1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexPosUv);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 画像のサイズを決める	
	float left = -anchorPoint_.x * size_.x;
	float right = (1.0f - anchorPoint_.x) * size_.x;
	float top = -anchorPoint_.y * size_.y;
	float bottom = (1.0f - anchorPoint_.y) * size_.y;

	// 頂点インデックス
	vertexData_[0].position = { left,bottom,0.0f,1.0f }; // 左下
	vertexData_[0].texcoord = { 0.0f,1.0f };
	vertexData_[1].position = { left,top,0.0f,1.0f }; // 左上
	vertexData_[1].texcoord = { 0.0f,0.0f };
	vertexData_[2].position = { right,bottom,0.0f,1.0f }; // 右下
	vertexData_[2].texcoord = { 1.0f,1.0f };
	vertexData_[3].position = { right,top,0.0f,1.0f }; // 右上
	vertexData_[3].texcoord = { 1.0f,0.0f };

	// Sprite用の頂点インデックスのリソースを作る
	indexResource_ = CreateBufferResource(device_, sizeof(uint32_t) * 6);
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	// 三角形
	indexDataSprite[0] = 0;  indexDataSprite[1] = 1;  indexDataSprite[2] = 2;
	// 三角形2
	indexDataSprite[3] = 1;  indexDataSprite[4] = 3;  indexDataSprite[5] = 2;
}

void Sprite::CreateConstBufferData(const Vector4& color) {
	// Sprite用の定数バッファリソースを作る
	constBufferResource_ = CreateBufferResource(device_, sizeof(ConstBufferData));
	// 書き込むためのアドレスを取得
	constBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&constBufferData_));
	// 色の設定
	constBufferData_->color = color;
	// UVTransform行列を初期化
	constBufferData_->uvTransform = MakeIdentity4x4();
	// wvp行列を初期化
	constBufferData_->WVP = Multiply(worldMatrix_, orthoMatrix_);
}