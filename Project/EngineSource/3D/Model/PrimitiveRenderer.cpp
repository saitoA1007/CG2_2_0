#include"PrimitiveRenderer.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/Math/MyMath.h"
#include"LogManager.h"
using namespace GameEngine;

ID3D12Device* PrimitiveRenderer::device_ = nullptr;
ID3D12GraphicsCommandList* PrimitiveRenderer::commandList_ = nullptr;
LinePSO* PrimitiveRenderer::linePSO_ = nullptr;

void PrimitiveRenderer::StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, LinePSO* linePSO) {
	device_ = device;
	commandList_ = commandList;
	linePSO_ = linePSO;
}

void PrimitiveRenderer::PreDraw() {
	// 線用のRootSignatureを設定。
	commandList_->SetGraphicsRootSignature(linePSO_->GetRootSignature());
	commandList_->SetPipelineState(linePSO_->GetPipelineState()); // PSOを設定   
}

PrimitiveRenderer* PrimitiveRenderer::Create() {

	// 線の作成を開始
	LogManager::GetInstance().Log("\nCreatePrimitiveModel");
	
	PrimitiveRenderer* Renderer = new PrimitiveRenderer();

	Renderer->totalVertices_ = 2;

	// トランスフォーメーション行列リソースを作成
	// TransformationMatrix用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	Renderer->transformMatrixResource_ = CreateBufferResource(device_, sizeof(TransformMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	Renderer->transformMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&Renderer->transformMatrixData_));
	// 単位行列を書き込んでおく
	Renderer->transformMatrixData_->VP = MakeIdentity4x4();

	// 無事に作成出来たことを伝える
	LogManager::GetInstance().Log("SuccessCreatedPrimitiveModel");

	return Renderer;
}

void PrimitiveRenderer::DrawLine3d(const LineMesh* lineMesh, const Matrix4x4& VPMatrix) {

	// カメラ座標を適応
	transformMatrixData_->VP = VPMatrix;

	commandList_->IASetVertexBuffers(0, 1, lineMesh->GetVertexBuffer());
	// 描画を線に設定
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	commandList_->SetGraphicsRootConstantBufferView(0, transformMatrixResource_->GetGPUVirtualAddress());
	// ドローコール
	commandList_->DrawInstanced(totalVertices_, 1, 0, 0);
}

