#include"LineMesh.h"
#include"CreateBufferResource.h"
using namespace GameEngine;

ID3D12Device* LineMesh::device_ = nullptr;

void LineMesh::StaticInitialize(ID3D12Device* device) {
	device_ = device;
}

void LineMesh::Initialize(const Vector3& p1, const Vector3& p2, const Vector4& color) {

	// 頂点バッファを作成
	// vertexResourceを作成
	vertexResource_ = CreateBufferResource(device_, sizeof(VertexPosColor) * 2);
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分(線を描画させる性質上先端が1ピクセル未満になってしまうのを回避するため)
	vertexBufferView_.SizeInBytes = sizeof(VertexPosColor) * 2;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexPosColor);

	// 頂点データを生成。 頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexPosColor_));
	// 2つの頂点テータを作成
	vertexPosColor_[0].pos = { p1.x,p1.y,p1.z,1.0f };
	vertexPosColor_[0].color = color;
	vertexPosColor_[1].pos = { p2.x,p2.y,p2.z,1.0f };
	vertexPosColor_[1].color = color;
}

void LineMesh::SetPosition(const Vector3& p1, const Vector3& p2) {
	vertexPosColor_[0].pos = { p1.x,p1.y,p1.z,1.0f };
	vertexPosColor_[1].pos = { p2.x,p2.y,p2.z,1.0f };
}

void LineMesh::SetColor(const Vector4& color) {
	vertexPosColor_[0].color = color;
	vertexPosColor_[1].color = color;
}
