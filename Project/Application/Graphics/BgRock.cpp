#include"BgRock.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void BgRock::Initialize() {
	// ワールド行列の初期化
	worldTransform_.Initialize({ { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f } });

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#endif
	// 値を適応させる
	ApplyDebugParam();
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
	GameParamEditor::GetInstance()->AddItem("BgRock", "Scale", worldTransform_.transform_.scale);
	GameParamEditor::GetInstance()->AddItem("BgRock", "Rotate", worldTransform_.transform_.rotate);
	GameParamEditor::GetInstance()->AddItem("BgRock", "Translate", worldTransform_.transform_.translate);
	
}

void BgRock::ApplyDebugParam() {
	worldTransform_.transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Scale");
	worldTransform_.transform_.rotate = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Rotate");
	worldTransform_.transform_.translate = GameParamEditor::GetInstance()->GetValue<Vector3>("BgRock", "Translate");
}