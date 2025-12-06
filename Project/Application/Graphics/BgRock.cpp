#include"BgRock.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void BgRock::Initialize(const uint32_t& texture) {
	// ワールド行列の初期化
	worldTransform_.Initialize({ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } });

	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = texture;

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#endif
	// 値を適応させる
	ApplyDebugParam();

	// 行列の更新処理
	worldTransform_.UpdateTransformMatrix();
}

void BgRock::Update() {
#ifdef _DEBUG
	// 値を適応させる
	ApplyDebugParam();
#endif

	// 行列の更新処理
	worldTransform_.UpdateTransformMatrix();
}

void BgRock::RegisterBebugParam() {
	int index = 0;
	GameParamEditor::GetInstance()->AddItem("BgRock", "Scale", worldTransform_.transform_.scale, index++);
	GameParamEditor::GetInstance()->AddItem("BgRock", "Rotate", worldTransform_.transform_.rotate, index++);
	GameParamEditor::GetInstance()->AddItem("BgRock", "Translate", worldTransform_.transform_.translate, index++);


	GameParamEditor::GetInstance()->AddItem("BgRock", "IceColor", iceMaterial_->materialData_->color);
	GameParamEditor::GetInstance()->AddItem("BgRock", "SpecularColor", specularColor);
	GameParamEditor::GetInstance()->AddItem("BgRock", "RimColor", rimColor);
	GameParamEditor::GetInstance()->AddItem("BgRock", "Shininess", iceMaterial_->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem("BgRock", "RimIntensity", iceMaterial_->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem("BgRock", "RimPower", iceMaterial_->materialData_->rimPower);
}

void BgRock::ApplyDebugParam() {
	worldTransform_.transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Scale");
	worldTransform_.transform_.rotate = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Rotate");
	worldTransform_.transform_.translate = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Translate");

	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>("BgRock", "IceColor");
	specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>("BgRock", "SpecularColor");
	rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>("BgRock", "RimColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>("BgRock", "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>("BgRock", "RimIntensity");
	iceMaterial_->materialData_->rimPower = GameParamEditor::GetInstance()->GetValue<float>("BgRock", "RimPower");


	iceMaterial_->materialData_->rimColor.x = rimColor.x;
	iceMaterial_->materialData_->rimColor.y = rimColor.y;
	iceMaterial_->materialData_->rimColor.z = rimColor.z;
	iceMaterial_->materialData_->specularColor.x = specularColor.x;
	iceMaterial_->materialData_->specularColor.y = specularColor.y;
	iceMaterial_->materialData_->specularColor.z = specularColor.z;
}