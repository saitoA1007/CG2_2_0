#include"TitleScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"GameParamEditor.h"
#include"CreateBufferResource.h"
using namespace GameEngine;

TitleScene::~TitleScene() {
}

void TitleScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("TitleScene");

#pragma endregion

	// メインカメラの初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720, context_->graphicsDevice->GetDevice());

	// 天球モデルを生成
	planeModel_ = context_->modelManager->GetNameByModel("Plane");
	worldTransform_.Initialize({ {5.0f,5.0f,5.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	worldTransform_.UpdateTransformMatrix();

	// コマンドリストを取得
	commandList_ = context->graphicsDevice->GetCommandList();

	// マテリアルリソースを作成
	// マテリアル用のリソースを作る。color1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(context->graphicsDevice->GetDevice(), sizeof(MaterialData));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// 白色に設定
	materialData_->color = {1.0f,1.0f,1.0f,1.0f};
	// UVTransform行列を初期化
	materialData_->uvTransform = MakeIdentity4x4();
	// テクスチャデータ
	materialData_->textureHandle = 0;
	// 時間
	materialData_->timer = 0.0f;

	srvManager_ = context->srvManager;
}

void TitleScene::Update() {

	// カメラの更新処理
	mainCamera_->Update();
}

void TitleScene::Draw(const bool& isDebugView) {

	// 描画に使用するカメラを設定
	if (isDebugView) {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	} else {
		// 描画に使用するカメラを設定
		ModelRenderer::SetCamera(mainCamera_->GetVPMatrix(), mainCamera_->GetCameraResource());
	}

	//===========================================================
	// 3D描画
	//===========================================================

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Test);

	// 平面描画
	{
		// カメラ座標に変換
		worldTransform_.SetWVPMatrix(planeModel_->GetLocalMatrix(), context_->debugCamera_->GetVPMatrix());

		// メッシュを取得
		const std::vector<std::unique_ptr<Mesh>>& meshes = planeModel_->GetMeshes();

		for (uint32_t i = 0; i < meshes.size(); ++i) {
			commandList_->IASetVertexBuffers(0, 1, &meshes[i]->GetVertexBufferView());
			commandList_->IASetIndexBuffer(&meshes[i]->GetIndexBufferView());
			commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// マテリアルが設定されていなければデフォルトのマテリアルを使う
			commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(2, srvManager_->GetSRVHeap()->GetGPUDescriptorHandleForHeapStart());
			commandList_->SetGraphicsRootConstantBufferView(1, worldTransform_.GetTransformResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootConstantBufferView(3, context_->debugCamera_->GetCameraResource()->GetGPUVirtualAddress());
			
			if (meshes[i]->GetTotalIndices() != 0) {
				commandList_->DrawIndexedInstanced(meshes[i]->GetTotalIndices(), 1, 0, 0, 0);
			} else {
				commandList_->DrawInstanced(meshes[i]->GetTotalVertices(), 1, 0, 0);
			}
		}
	}
}
