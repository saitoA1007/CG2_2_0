#include"DrawTaskModel.h"
#include"EngineSource/2D/ImGuiManager.h"

using namespace GameEngine;

DrawTaskModel::~DrawTaskModel() {
	delete sprite_;
}

void DrawTaskModel::Initialize(const uint32_t& uvCheckerGH, const uint32_t& whiteGH) {

	// 画像を読み込み
	uvCheckerGH_ = uvCheckerGH;
	whiteGH_ = whiteGH;

	// スプライト
	sprite_ = Sprite::Create({0.0f,0.0f},{640.0f,360.0f},{1.0f,1.0f,1.0f,1.0f});

	// 平面
	planeTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	planeMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	planeMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// 球
	sphereTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	sphereMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	sphereMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// ティーポッド
	utahTeapotTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	teapotMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	teapotMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// ウサギ
	bunnyTransform_ = { {1.0f,1.0f,1.0f},{0.0f,2.8f,0.0f},{0.0f,0.0f,0.0f} };
	bunnyMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	bunnyMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// スザンヌ
	suzanneTransform_ = { {1.0f,1.0f,1.0f},{0.0f,3.1f,0.0f},{0.0f,0.0f,0.0f} };
	suzanneMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	suzanneMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// マルチメッシュ
	multiMeshTransform_ = { {1.0f,1.0f,1.0f},{0.0f,3.1f,0.0f},{0.0f,0.0f,0.0f} };
	multiMeshMaterial_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	multiMeshMaterial_.color = { 1.0f,1.0f,1.0f,1.0f };
	// マルチマテリアル
	multiMaterialTransform_ = { {1.0f,1.0f,1.0f},{0.0f,3.1f,0.0f},{0.0f,0.0f,0.0f} };
	// マテリアルの初期化
	multiMaterial_[0].uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	multiMaterial_[0].color = { 1.0f,1.0f,1.0f,1.0f };
	multiMaterial_[1].uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	multiMaterial_[1].color = { 1.0f,1.0f,1.0f,1.0f };

	// 行列の初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 複数モデル描画用の平面モデル
	secondPlaneWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-2.7f,0.0f,0.0f} });
	secondPlaneMaterial_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 500.0f, true);
	secondPlaneMaterialData_.uvTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	secondPlaneMaterialData_.color = { 1.0f,1.0f,1.0f,1.0f };
}

void DrawTaskModel::Update() {

	// スプライト
	sprite_->SetPosition(spritePos_);
	sprite_->SetUvMatrix(uvSpriteTransform_);

	// 行列の更新処理
	worldTransform_.UpdateTransformMatrix();
	secondPlaneWorldTransform_.UpdateTransformMatrix();
}

void DrawTaskModel::Draw2D() {
	// スプライトを描画
	if (isDrawSprite_) {
		sprite_->Draw(uvCheckerGH_);
	}
}

void DrawTaskModel::Draw3D(const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource) {

	switch (selectModelNum_) {

	case ModelMode::plane:
		// 平面
		if (isLightOn_) {
			planeModel_->DrawLight(lightGroupResource, cameraResource);
		}
		planeModel_->Draw(worldTransform_, VPMatrix);
		break;

	case ModelMode::sphere:
		// 球
		if (isLightOn_) {
			sphereModel_->DrawLight(lightGroupResource, cameraResource);
		}
		sphereModel_->Draw(worldTransform_, uvCheckerGH_, VPMatrix);
		break;

	case ModelMode::UtahTeapot:
		// ティーポッド
		if (isLightOn_) {
			utahTeapotModel_->DrawLight(lightGroupResource, cameraResource);
		}
		utahTeapotModel_->Draw(worldTransform_, VPMatrix);
		break;

	case ModelMode::StanfordBunny:
		// ウサギ
		if (isLightOn_) {
			bunnyModel_->DrawLight(lightGroupResource, cameraResource);
		}
		bunnyModel_->Draw(worldTransform_, VPMatrix);
		break;

	case ModelMode::Suzanne:
		// スザンヌ
		if (isLightOn_) {
			suzanneModel_->DrawLight(lightGroupResource, cameraResource);
		}
		suzanneModel_->Draw(worldTransform_, whiteGH_, VPMatrix);
		break;

	case ModelMode::MultiMesh:
		// マルチメッシュ
		if (isLightOn_) {
			multiMeshModel_->DrawLight(lightGroupResource, cameraResource);
		}
		multiMeshModel_->Draw(worldTransform_, VPMatrix);
		break;

	case ModelMode::MultiMaterial:
		// マルチマテリアル
		if (isLightOn_) {
			multiMaterialModel_->Draw(worldTransform_, VPMatrix, lightGroupResource, cameraResource);
		}
		multiMaterialModel_->Draw(worldTransform_, VPMatrix);
		break;
	}

	// 複数描画用の平面モデルを描画
	if (isLightOn_) {
		planeModel_->DrawLight(lightGroupResource, cameraResource);
	}
	planeModel_->Draw(secondPlaneWorldTransform_, VPMatrix, &secondPlaneMaterial_);
}

void DrawTaskModel::DebugWindow() {

	if (ImGui::TreeNodeEx("Sprite", ImGuiTreeNodeFlags_Framed)) {
		ImGui::DragFloat2("Position", &spritePos_.x, 1.0f);

		if (ImGui::TreeNode("UV")) {
			ImGui::DragFloat2("Scale", &uvSpriteTransform_.scale.x, 0.01f);
			ImGui::DragFloat2("Rotate", &uvSpriteTransform_.rotate.x, 0.01f);
			ImGui::DragFloat2("Translate", &uvSpriteTransform_.translate.x, 0.01f);
			ImGui::TreePop();
		}

		ImGui::Checkbox("IsDrawSprite", &isDrawSprite_);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("3DModel", ImGuiTreeNodeFlags_Framed)) {

		// 通常のモデル
		if (ImGui::TreeNodeEx("Object 1", ImGuiTreeNodeFlags_Framed)) {
			// SRT
			DebugModelSRT();
			// マテリアル
			DebugModelMaterial();
			ImGui::TreePop();
		}
		
		// 複数描画用の平面モデル
		if (ImGui::TreeNodeEx("Object 2", ImGuiTreeNodeFlags_Framed)) {

			// モデルSRT
			ImGui::DragFloat3("Scale", &secondPlaneWorldTransform_.transform_.scale.x, 0.01f);
			ImGui::DragFloat3("Rotate", &secondPlaneWorldTransform_.transform_.rotate.x, 0.01f);
			ImGui::DragFloat3("Translate", &secondPlaneWorldTransform_.transform_.translate.x, 0.01f);

			// マテリアル
			if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_Framed)) {
				ImGui::DragFloat3("uvScale", &secondPlaneMaterialData_.uvTransform.scale.x, 0.01f);
				ImGui::DragFloat3("uvRotate", &secondPlaneMaterialData_.uvTransform.rotate.x, 0.01f);
				ImGui::DragFloat3("uvTranslate", &secondPlaneMaterialData_.uvTransform.translate.x, 0.01f);
				secondPlaneMaterial_.SetUVTransform(secondPlaneMaterialData_.uvTransform);
				ImGui::ColorEdit3("Color", &secondPlaneMaterialData_.color.x);
				secondPlaneMaterial_.SetColor(secondPlaneMaterialData_.color);
				ImGui::TreePop();	
			}
			ImGui::TreePop();
		}

		// ライティング
		if (ImGui::Checkbox("isEnableLight", &isLightOn_)) {

			if (isLightOn_) {
				// 平面モデル
				planeModel_->SetDefaultIsEnableLight(true);
				// 球モデル
				sphereModel_->SetDefaultIsEnableLight(true);
				// ティーポッドモデル
				utahTeapotModel_->SetDefaultIsEnableLight(true);
				// ウサギモデル
				bunnyModel_->SetDefaultIsEnableLight(true);
				// スザンヌモデル
				suzanneModel_->SetDefaultIsEnableLight(true);
				// マルチメッシュモデル
				multiMeshModel_->SetDefaultIsEnableLight(true);
				// マルチマテリアル
				multiMaterialModel_->SetDefaultIsEnableLight(true, 0);
				multiMaterialModel_->SetDefaultIsEnableLight(true, 1);

				// 複数描画用の平面モデル
				secondPlaneMaterial_.SetEnableLighting(true);
			} else {
				planeModel_->SetDefaultIsEnableLight(false);
				// 球モデル
				sphereModel_->SetDefaultIsEnableLight(false);
				// ティーポッドモデル
				utahTeapotModel_->SetDefaultIsEnableLight(false);
				// ウサギモデル
				bunnyModel_->SetDefaultIsEnableLight(false);
				// スザンヌモデル
				suzanneModel_->SetDefaultIsEnableLight(false);
				// マルチメッシュモデル
				multiMeshModel_->SetDefaultIsEnableLight(false);
				// マルチマテリアル
				multiMaterialModel_->SetDefaultIsEnableLight(false, 0);
				multiMaterialModel_->SetDefaultIsEnableLight(false, 1);

				// 複数描画用の平面モデル
				secondPlaneMaterial_.SetEnableLighting(false);
			}	
		}
		ImGui::TreePop();
	}
}

void DrawTaskModel::DebugModelSRT() {
	// 描画モデルを切り替え
	ImGui::Combo("ModelMode", &selectModelNum_, kModelName_, IM_ARRAYSIZE(kModelName_));

	// モデルのSRT
	switch (selectModelNum_) {
	case ModelMode::plane:
		ImGui::DragFloat3("Scale", &planeTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &planeTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &planeTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = planeTransform_;
		break;

	case ModelMode::sphere:
		ImGui::DragFloat3("Scale", &sphereTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &sphereTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &sphereTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = sphereTransform_;
		break;

	case ModelMode::UtahTeapot:
		ImGui::DragFloat3("Scale", &utahTeapotTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &utahTeapotTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &utahTeapotTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = utahTeapotTransform_;
		break;

	case ModelMode::StanfordBunny:
		ImGui::DragFloat3("Scale", &bunnyTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &bunnyTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &bunnyTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = bunnyTransform_;
		break;

	case ModelMode::Suzanne:
		ImGui::DragFloat3("Scale", &suzanneTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &suzanneTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &suzanneTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = suzanneTransform_;
		break;

	case ModelMode::MultiMesh:
		ImGui::DragFloat3("Scale", &multiMeshTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &multiMeshTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &multiMeshTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = multiMeshTransform_;
		break;

	case ModelMode::MultiMaterial:
		ImGui::DragFloat3("Scale", &multiMaterialTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &multiMaterialTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &multiMaterialTransform_.translate.x, 0.01f);
		worldTransform_.transform_ = multiMaterialTransform_;
		break;
	}
}

void DrawTaskModel::DebugModelMaterial() {
	if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_Framed)) {

		switch (selectModelNum_) {
		case ModelMode::plane:
			ImGui::DragFloat3("uvScale", &planeMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &planeMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &planeMaterial_.uvTransform.translate.x, 0.01f);
			planeModel_->SetDefaultUVMatrix(planeMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &planeMaterial_.color.x);
			planeModel_->SetDefaultColor(planeMaterial_.color, 0);
			break;

		case ModelMode::sphere:
			ImGui::DragFloat3("uvScale", &sphereMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &sphereMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &sphereMaterial_.uvTransform.translate.x, 0.01f);
			sphereModel_->SetDefaultUVMatrix(sphereMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &sphereMaterial_.color.x);
			sphereModel_->SetDefaultColor(sphereMaterial_.color, 0);
			break;

		case ModelMode::UtahTeapot:
			ImGui::DragFloat3("uvScale", &teapotMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &teapotMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &teapotMaterial_.uvTransform.translate.x, 0.01f);
			utahTeapotModel_->SetDefaultUVMatrix(teapotMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &teapotMaterial_.color.x);
			utahTeapotModel_->SetDefaultColor(teapotMaterial_.color, 0);
			break;

		case ModelMode::StanfordBunny:
			ImGui::DragFloat3("uvScale", &bunnyMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &bunnyMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &bunnyMaterial_.uvTransform.translate.x, 0.01f);
			bunnyModel_->SetDefaultUVMatrix(bunnyMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &bunnyMaterial_.color.x);
			bunnyModel_->SetDefaultColor(bunnyMaterial_.color, 0);
			break;

		case ModelMode::Suzanne:
			ImGui::DragFloat3("uvScale", &suzanneMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &suzanneMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &suzanneMaterial_.uvTransform.translate.x, 0.01f);
			suzanneModel_->SetDefaultUVMatrix(suzanneMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &suzanneMaterial_.color.x);
			suzanneModel_->SetDefaultColor(suzanneMaterial_.color, 0);
			break;

		case ModelMode::MultiMesh:
			ImGui::DragFloat3("uvScale", &multiMeshMaterial_.uvTransform.scale.x, 0.01f);
			ImGui::DragFloat3("uvRotate", &multiMeshMaterial_.uvTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("uvTranslate", &multiMeshMaterial_.uvTransform.translate.x, 0.01f);
			multiMeshModel_->SetDefaultUVMatrix(multiMeshMaterial_.uvTransform, 0);
			ImGui::ColorEdit3("Color", &multiMeshMaterial_.color.x);
			multiMeshModel_->SetDefaultColor(multiMeshMaterial_.color, 0);
			break;

		case ModelMode::MultiMaterial:
			if (ImGui::TreeNode("Material : 0")) {
				ImGui::DragFloat3("uvScale", &multiMaterial_[0].uvTransform.scale.x, 0.01f);
				ImGui::DragFloat3("uvRotate", &multiMaterial_[0].uvTransform.rotate.x, 0.01f);
				ImGui::DragFloat3("uvTranslate", &multiMaterial_[0].uvTransform.translate.x, 0.01f);
				multiMaterialModel_->SetDefaultUVMatrix(multiMaterial_[0].uvTransform, 0);
				ImGui::ColorEdit3("Color", &multiMaterial_[0].color.x);
				multiMaterialModel_->SetDefaultColor(multiMaterial_[0].color, 0);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Material : 1")) {
				ImGui::DragFloat3("uvScale", &multiMaterial_[1].uvTransform.scale.x, 0.01f);
				ImGui::DragFloat3("uvRotate", &multiMaterial_[1].uvTransform.rotate.x, 0.01f);
				ImGui::DragFloat3("uvTranslate", &multiMaterial_[1].uvTransform.translate.x, 0.01f);
				multiMaterialModel_->SetDefaultUVMatrix(multiMaterial_[1].uvTransform, 1);
				ImGui::ColorEdit3("Color", &multiMaterial_[1].color.x);
				multiMaterialModel_->SetDefaultColor(multiMaterial_[1].color, 1);
				ImGui::TreePop();
			}
			break;
		}
		ImGui::TreePop();
	}
}