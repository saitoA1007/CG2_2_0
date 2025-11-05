#include"ModelRenderer.h"
#include<cassert>

using namespace GameEngine;

ID3D12Device* ModelRenderer::device_ = nullptr;
ID3D12GraphicsCommandList* ModelRenderer::commandList_ = nullptr;
TextureManager* ModelRenderer::textureManager_ = nullptr;
std::unordered_map<RenderMode, DrawPsoData> ModelRenderer::psoList_;

void ModelRenderer::StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager, PSOManager* psoManager) {
	device_ = device;
	commandList_ = commandList;
	textureManager_ = textureManager;

	// 通常描画のpsoデータを取得する
	psoList_[RenderMode::DefaultModel] = psoManager->GetDrawPsoData("Default3D");
	// インスタンシング描画用のデータを取得する
	psoList_[RenderMode::Instancing] = psoManager->GetDrawPsoData("Instancing3D");
	// グリッド描画用のデータを取得する
	psoList_[RenderMode::Grid] = psoManager->GetDrawPsoData("Grid");
	// アニメーション描画用のデータを取得する
	psoList_[RenderMode::AnimationModel] = psoManager->GetDrawPsoData("Animation");
}

void ModelRenderer::PreDraw(RenderMode mode) {
	auto pso = psoList_.find(mode);
	if (pso == psoList_.end()) {
		assert(0);
		return;
	}

	commandList_->SetGraphicsRootSignature(pso->second.rootSignature);
	commandList_->SetPipelineState(pso->second.graphicsPipelineState);
}

void ModelRenderer::Draw(const Model* model, WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material) {

	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransform.SetWVPMatrix(model->GetLocalMatrix(), VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void ModelRenderer::Draw(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransform.SetWVPMatrix(model->GetLocalMatrix(), VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void ModelRenderer::Draw(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource, const Material* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransform.SetWVPMatrix(model->GetLocalMatrix(), VPMatrix);
	} else {
		worldTransform.SetWVPMatrix(VPMatrix);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
		commandList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void ModelRenderer::Draw(const Model* model, const uint32_t& numInstance, WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransforms.SetWVPMatrix(numInstance, model->GetLocalMatrix(), VPMatrix);
	} else {
		worldTransforms.SetWVPMatrix(numInstance, VPMatrix);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootDescriptorTable(1, *worldTransforms.GetInstancingSrvGPU());
		commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(textureHandle));

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void ModelRenderer::Draw(const Model* model, const uint32_t& numInstance, WorldTransforms& worldTransforms, const Matrix4x4& VPMatrix, const Material* material) {
	// カメラ座標に変換
	if (model->IsLoad()) {
		worldTransforms.SetWVPMatrix(numInstance, model->GetLocalMatrix(), VPMatrix);
	} else {
		worldTransforms.SetWVPMatrix(numInstance, VPMatrix);
	}

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {
		commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootDescriptorTable(1, *worldTransforms.GetInstancingSrvGPU());

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), numInstance, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), numInstance, 0, 0);
		}
	}
}

void ModelRenderer::DrawAnimation(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const SkinCluster& skinCluster, const Material* material) {
	// カメラ座標に変換
	worldTransform.SetWVPMatrix(VPMatrix);

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	for (uint32_t i = 0; i < meshes.size(); ++i) {

		D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
			meshes[i]->GetVertexBufferView(),
			skinCluster.influenceBufferView
		};

		commandList_->IASetVertexBuffers(0, 2, vbvs);
		commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// マテリアルを設定
		const Material* drawMaterial = model->GetMaterial(meshes[i]->GetMaterialName());

		// マテリアルが設定されていなければデフォルトのマテリアルを使う
		if (material == nullptr) {
			commandList_->SetGraphicsRootConstantBufferView(0, drawMaterial->GetMaterialResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandlesGPU(drawMaterial->GetTextureHandle()));
		} else {
			commandList_->SetGraphicsRootConstantBufferView(0, material->GetMaterialResource()->GetGPUVirtualAddress());
		}
		commandList_->SetGraphicsRootConstantBufferView(1, worldTransform.GetTransformResource()->GetGPUVirtualAddress());

		commandList_->SetGraphicsRootDescriptorTable(3, skinCluster.paletteSrvHandle.second);

		if (meshes[i]->GetTotalIndices() != 0) {
			commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
		} else {
			commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
		}
	}
}

void ModelRenderer::DrawGrid(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource) {

	worldTransform.SetWVPMatrix(VPMatrix);

	// メッシュを取得
	const std::vector<std::unique_ptr<Mesh>>& meshes = model->GetMeshes();

	commandList_->IASetVertexBuffers(0, 1, &meshes[0]->GetVertexBufferView());
	commandList_->IASetIndexBuffer(&meshes[0]->GetIndexBufferView());
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->SetGraphicsRootConstantBufferView(0, worldTransform.GetTransformResource()->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(1, cameraResource->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(meshes[0]->GetTotalIndices(), 1, 0, 0, 0);
}

void ModelRenderer::DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource) {
	commandList_->SetGraphicsRootConstantBufferView(3, lightGroupResource->GetGPUVirtualAddress());
	commandList_->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
}