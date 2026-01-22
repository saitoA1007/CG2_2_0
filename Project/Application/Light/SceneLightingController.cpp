#include"SceneLightingController.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void SceneLightingController::Initialize(ID3D12Device* device) {

	// 平行光源ライト
	directionalData_.active = true;
	directionalData_.color = { 1.0f,1.0f,1.0f,1.0f };
	directionalData_.direction = { 0.0,-1.0f,0.0f };
	directionalData_.intensity = 1.0f;

	// 点光源
	for (int i = 0; i < pointLightDatas_.size(); ++i) {
		pointLightDatas_[i].active = true;
		pointLightDatas_[i].color = { 1.0f,1.0f,1.0f,1.0f };
		pointLightDatas_[i].position = { 2.0f,0.0f,0.0f };
		pointLightDatas_[i].radius = 2.0f;
		pointLightDatas_[i].decay = 1.0f;
		pointLightDatas_[i].intensity = 1.0f;
	}

	// スポットライト
	for (int i = 0; i < spotLightDatas_.size(); ++i) {
		spotLightDatas_[i].active = true;
		spotLightDatas_[i].color = { 1.0f,1.0f,1.0f,1.0f };
		spotLightDatas_[i].position = { 0.0f,1.0f,0.0f };
		spotLightDatas_[i].direction = { 0.0f,-1.0f,0.0f };
		spotLightDatas_[i].distance = 10.0f;
		spotLightDatas_[i].intensity = 1.0f;
		spotLightDatas_[i].cosAngle = 0.2f;
		spotLightDatas_[i].cosFalloffStart = 1.0f;
		spotLightDatas_[i].decay = 1.0f;
	}

	// 面光源
	for (int i = 0; i < areaLightDatas_.size(); ++i) {
		areaLightDatas_[i].color = {1.0f,1.0f,1.0f,1.0f};
		areaLightDatas_[i].position = {0.0f,0.0f,-1.0f};
		areaLightDatas_[i].intensity = 1.0f;
		areaLightDatas_[i].right = { 1.0f,0.0f,0.0f };
		areaLightDatas_[i].width = 5.0f;
		areaLightDatas_[i].up = { 0.0f,1.0f,0.0f };
		areaLightDatas_[i].height = 5.0f;
		areaLightDatas_[i].distance = 10.0f;
		areaLightDatas_[i].decay = 0.5f;
		areaLightDatas_[i].active = true;
	}
	
	// ライトの設定
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(device, true, 2, 2,2);
	lightManager_->SetDirectionalData(directionalData_);
	lightManager_->SetPointData(pointLightDatas_[0], 0);
	lightManager_->SetPointData(pointLightDatas_[1], 1);
	lightManager_->SetSpotData(spotLightDatas_[0], 0);
	lightManager_->SetSpotData(spotLightDatas_[1], 1);
	lightManager_->SetAreaData(areaLightDatas_[0], 0);
	lightManager_->SetAreaData(areaLightDatas_[1], 1);

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void SceneLightingController::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	lightManager_->Update();
}

void SceneLightingController::RegisterBebugParam() {
	// 平行光源の設定
	GameParamEditor::GetInstance()->AddItem(directinalName_, "Direction", directionalData_.direction);
	GameParamEditor::GetInstance()->AddItem(directinalName_, "Intensity", directionalData_.intensity);
	GameParamEditor::GetInstance()->AddItem(directinalName_, "Color", directionalData_.color);

	// 点光源の設定
	for (int i = 0; i < pointLightDatas_.size(); ++i) {
		std::string name = pointName_ + std::to_string(i);
		GameParamEditor::GetInstance()->AddItem(name, "Color", pointLightDatas_[i].color);
		GameParamEditor::GetInstance()->AddItem(name, "Position", pointLightDatas_[i].position);
		GameParamEditor::GetInstance()->AddItem(name, "Radius", pointLightDatas_[i].radius);
		GameParamEditor::GetInstance()->AddItem(name, "Decay", pointLightDatas_[i].decay);
		GameParamEditor::GetInstance()->AddItem(name, "Intensity", pointLightDatas_[i].intensity);
	}

	// スポットライトの設定
	for (int i = 0; i < spotLightDatas_.size(); ++i) {
		std::string name = spotName_ + std::to_string(i);
		GameParamEditor::GetInstance()->AddItem(name, "Color", spotLightDatas_[i].color);
		GameParamEditor::GetInstance()->AddItem(name, "Position", spotLightDatas_[i].position);
		GameParamEditor::GetInstance()->AddItem(name, "Direction", spotLightDatas_[i].direction);
		GameParamEditor::GetInstance()->AddItem(name, "Distance", spotLightDatas_[i].distance);
		GameParamEditor::GetInstance()->AddItem(name, "Intensity", spotLightDatas_[i].intensity);
		GameParamEditor::GetInstance()->AddItem(name, "CosAngle", spotLightDatas_[i].cosAngle);
		GameParamEditor::GetInstance()->AddItem(name, "CosFalloffStart", spotLightDatas_[i].cosFalloffStart);
		GameParamEditor::GetInstance()->AddItem(name, "Decay", spotLightDatas_[i].decay);
	}

	// 面光源の設定
	for (int i = 0; i < areaLightDatas_.size(); ++i) {
		std::string name = areaName_ + std::to_string(i);
		GameParamEditor::GetInstance()->AddItem(name, "Color", areaLightDatas_[i].color);
		GameParamEditor::GetInstance()->AddItem(name, "Position", areaLightDatas_[i].position);
		GameParamEditor::GetInstance()->AddItem(name, "Intensity", areaLightDatas_[i].intensity);
		GameParamEditor::GetInstance()->AddItem(name, "Right", areaLightDatas_[i].right);
		GameParamEditor::GetInstance()->AddItem(name, "Width", areaLightDatas_[i].width);
		GameParamEditor::GetInstance()->AddItem(name, "Up", areaLightDatas_[i].up);
		GameParamEditor::GetInstance()->AddItem(name, "Height", areaLightDatas_[i].height);
		GameParamEditor::GetInstance()->AddItem(name, "Distance", areaLightDatas_[i].distance);
		GameParamEditor::GetInstance()->AddItem(name, "Decay", areaLightDatas_[i].decay);
	}
}

void SceneLightingController::ApplyDebugParam(){
	// 平行光源の適応
	Vector3 tmpDireciton = GameParamEditor::GetInstance()->GetValue<Vector3>(directinalName_, "Direction");
	directionalData_.direction = Normalize(tmpDireciton);
	directionalData_.intensity = GameParamEditor::GetInstance()->GetValue<float>(directinalName_, "Intensity");
	directionalData_.color = GameParamEditor::GetInstance()->GetValue<Vector4>(directinalName_, "Color");

	// 点光源の適応
	for (int i = 0; i < pointLightDatas_.size(); ++i) {
		std::string name = pointName_ + std::to_string(i);
		pointLightDatas_[i].color = GameParamEditor::GetInstance()->GetValue<Vector4>(name, "Color");
		pointLightDatas_[i].position = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Position");
		pointLightDatas_[i].radius = GameParamEditor::GetInstance()->GetValue<float>(name, "Radius");
		pointLightDatas_[i].decay = GameParamEditor::GetInstance()->GetValue<float>(name, "Decay");
		pointLightDatas_[i].intensity = GameParamEditor::GetInstance()->GetValue<float>(name, "Intensity");
	}

	// スポットライトの適応
	for (int i = 0; i < spotLightDatas_.size(); ++i) {
		std::string name = spotName_ + std::to_string(i);
		spotLightDatas_[i].color = GameParamEditor::GetInstance()->GetValue<Vector4>(name, "Color");
		spotLightDatas_[i].position = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Position");
		spotLightDatas_[i].direction = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Direction");
		spotLightDatas_[i].distance = GameParamEditor::GetInstance()->GetValue<float>(name, "Distance");
		spotLightDatas_[i].intensity = GameParamEditor::GetInstance()->GetValue<float>(name, "Intensity");
		spotLightDatas_[i].cosAngle = GameParamEditor::GetInstance()->GetValue<float>(name, "CosAngle");
		spotLightDatas_[i].cosFalloffStart = GameParamEditor::GetInstance()->GetValue<float>(name, "CosFalloffStart");
		spotLightDatas_[i].decay = GameParamEditor::GetInstance()->GetValue<float>(name, "Decay");
	}

	for (int i = 0; i < areaLightDatas_.size(); ++i) {
		std::string name = areaName_ + std::to_string(i);
		areaLightDatas_[i].color =  GameParamEditor::GetInstance()->GetValue<Vector4>(name, "Color");
		areaLightDatas_[i].position = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Position");
		areaLightDatas_[i].intensity = GameParamEditor::GetInstance()->GetValue<float>(name, "Intensity");
		areaLightDatas_[i].right = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Right");
		areaLightDatas_[i].width = GameParamEditor::GetInstance()->GetValue<float>(name, "Width");
		areaLightDatas_[i].up = GameParamEditor::GetInstance()->GetValue<Vector3>(name, "Up");
		areaLightDatas_[i].height = GameParamEditor::GetInstance()->GetValue<float>(name, "Height");
		areaLightDatas_[i].distance = GameParamEditor::GetInstance()->GetValue<float>(name, "Distance");
		areaLightDatas_[i].decay = GameParamEditor::GetInstance()->GetValue<float>(name, "Decay");
	}

	// ライトマネージャーに適応
	lightManager_->SetDirectionalData(directionalData_);
	lightManager_->SetPointData(pointLightDatas_[0], 0);
	lightManager_->SetPointData(pointLightDatas_[1], 1);
	lightManager_->SetSpotData(spotLightDatas_[0], 0);
	lightManager_->SetSpotData(spotLightDatas_[1], 1);
	lightManager_->SetAreaData(areaLightDatas_[0], 0);
	lightManager_->SetAreaData(areaLightDatas_[1], 1);
}