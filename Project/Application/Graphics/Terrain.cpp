#include"Terrain.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void Terrain::Initialize(const uint32_t& baseTexture, const uint32_t& iceTexture, const uint32_t& iceNormalTex) {
	// ワールド行列の初期化
	worldTransform_.Initialize({ { 30.0f,30.0f,30.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.1f,0.0f } });

	// マテリアルの初期化
	iceMaterial_ = std::make_unique<IceMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = iceTexture;
	iceMaterial_->materialData_->normalTextureHandle = iceNormalTex;
	iceMaterial_->materialData_->baseTextureHandle = baseTexture;

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void Terrain::Update() {
#ifdef _DEBUG
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void Terrain::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem("IceTerrain", "BaseColor", iceMaterial_->materialData_->baseColor);
	GameParamEditor::GetInstance()->AddItem("IceTerrain", "IceColor", iceMaterial_->materialData_->color);
	GameParamEditor::GetInstance()->AddItem("IceTerrain", "Shininess", iceMaterial_->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem("IceTerrain", "RimIntensity", iceMaterial_->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem("IceTerrain", "Time", iceMaterial_->materialData_->time);
	//iceMaterial_->materialData_->specularColor
	//iceMaterial_->materialData_->rimColor
	//iceMaterial_->materialData_->time
}

void Terrain::ApplyDebugParam() {
	iceMaterial_->materialData_->baseColor = GameParamEditor::GetInstance()->GetValue<Vector4>("IceTerrain", "BaseColor");
	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>("IceTerrain", "IceColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>("IceTerrain", "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>("IceTerrain", "RimIntensity");
	iceMaterial_->materialData_->time = GameParamEditor::GetInstance()->GetValue<float>("IceTerrain", "Time");
}