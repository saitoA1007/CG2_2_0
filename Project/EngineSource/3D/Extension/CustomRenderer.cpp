#include"CustomRenderer.h"
#include<cassert>

// 使用するマテリアル
#include"CustomMaterial/IceMaterial.h"

using namespace GameEngine;

ID3D12GraphicsCommandList* CustomRenderer::commandList_ = nullptr;
std::unordered_map<CustomRenderMode, DrawPsoData> CustomRenderer::psoList_;
Matrix4x4 CustomRenderer::vpMatrix_ = {};
ID3D12Resource* CustomRenderer::cameraResource_ = nullptr;
SrvManager* CustomRenderer::srvManager_ = nullptr;

void CustomRenderer::StaticInitialize(ID3D12Device* device,ID3D12GraphicsCommandList* commandList, SrvManager* srvManager, PSOManager* psoManager) {
	commandList_ = commandList;
	srvManager_ = srvManager;

	// 使用するマテリアルの静的初期化
	IceMaterial::StaticInitialize(device);
	IceRockMaterial::StaticInitialize(device);
	BossMaterial::StaticInitialize(device);

	// 氷描画のpsoデータを取得する
	psoList_[CustomRenderMode::Ice] = psoManager->GetDrawPsoData("IceMaterial");
	psoList_[CustomRenderMode::Rock] = psoManager->GetDrawPsoData("IceRock");
	psoList_[CustomRenderMode::RockBack] = psoManager->GetDrawPsoData("IceRockBack");
	psoList_[CustomRenderMode::RockBoth] = psoManager->GetDrawPsoData("IceRockBoth");
	psoList_[CustomRenderMode::BossAnimation] = psoManager->GetDrawPsoData("BossAnimation");
}

void CustomRenderer::PreDraw(CustomRenderMode mode) {
	auto pso = psoList_.find(mode);
	if (pso == psoList_.end()) {
		assert(0);
		return;
	}

	commandList_->SetGraphicsRootSignature(pso->second.rootSignature);
	commandList_->SetPipelineState(pso->second.graphicsPipelineState);
}

void CustomRenderer::SetCamera(const Matrix4x4& vpMatrix, ID3D12Resource* cameraResource) {
	vpMatrix_ = vpMatrix;
	cameraResource_ = cameraResource;
}

void CustomRenderer::DrawIce(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource, ICustomMaterial* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransform.SetWVPMatrix(model->GetLocalMatrix(), vpMatrix_);
	} else {
		worldTransform.SetWVPMatrix(vpMatrix_);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
			meshes[i]->GetVertexBufferView(),
			model->normalMapData_->tangentBufferView
		};
		commandList_->IASetVertexBuffers(0, 2, vbvs);
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList_->SetGraphicsRootConstantBufferView(0, material->GetResource()->GetGPUVirtualAddress());

		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
		commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void CustomRenderer::DrawRock(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource, IceRockMaterial* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransform.SetWVPMatrix(model->GetLocalMatrix(), vpMatrix_);
	} else {
		worldTransform.SetWVPMatrix(vpMatrix_);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList_->SetGraphicsRootConstantBufferView(0, material->GetResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
		commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void CustomRenderer::DrawAnimationWithLight(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource, BossMaterial* material) {

	// カメラ座標に変換
	worldTransform.SetWVPMatrix(vpMatrix_);

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
			meshes[i]->GetVertexBufferView(),
			model->skinClusterBron_->influenceBufferView
		};

		commandList_->IASetVertexBuffers(0, 2, vbvs);
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList_->SetGraphicsRootConstantBufferView(0, material->GetResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
		commandList_->SetGraphicsRootDescriptorTable(3, model->skinClusterBron_->paletteSrvHandle.second);
		commandList_->SetGraphicsRootConstantBufferView(4, lightGroupResource->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(5, cameraResource_->GetGPUVirtualAddress());

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}

}
