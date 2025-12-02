#include "StageWallPlane.h"
#include"EngineSource/3D/Extension/CustomMaterial/IceMaterial.h"
#include"GameParamEditor.h"

void StageWallPlane::Initialilze(const Transform &transform) {
	// ワールド変換を初期化
	worldTransform_.Initialize(transform);

	// 生存フラグを立てる
	isAlive_ = true;
}

void StageWallPlane::Update() {
	// ワールド行列の更新
	worldTransform_.UpdateTransformMatrix();
}

void StageWallPlane::RegisterDebugParam(IceMaterial *material) {
	// グループ名を固定
	const std::string group = "StageWallPlane";
	// マテリアルに関するパラメータを登録
	GameParamEditor::GetInstance()->AddItem(group, "BaseColor", material->materialData_->baseColor);
	GameParamEditor::GetInstance()->AddItem(group, "IceColor", material->materialData_->color);
    GameParamEditor::GetInstance()->AddItem(group, "SpecularColor", material->materialData_->specularColor);
	GameParamEditor::GetInstance()->AddItem(group, "RimColor", material->materialData_->rimColor);
	GameParamEditor::GetInstance()->AddItem(group, "Shininess", material->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem(group, "RimIntensity", material->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem(group, "Time", material->materialData_->time);
}

void StageWallPlane::ApplyDebugParam(IceMaterial* material) {
	if (!material) return;
	const std::string group = "StageWallPlane";
	// 適用
	material->materialData_->baseColor = GameParamEditor::GetInstance()->GetValue<Vector4>(group, "BaseColor");
	material->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>(group, "IceColor");
	Vector3 spec = GameParamEditor::GetInstance()->GetValue<Vector3>(group, "SpecularColor");
	material->materialData_->specularColor = spec;
	material->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>(group, "Shininess");
	Vector3 rim = GameParamEditor::GetInstance()->GetValue<Vector3>(group, "RimColor");
	material->materialData_->rimColor = rim;
	material->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>(group, "RimIntensity");
}