#include"Wall.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void Wall::Initialize(const Transform& transform) {

	// ワールド行列を初期化
	Transform tmpTransform = transform;
	tmpTransform.scale.x = 1.0f;
	tmpTransform.scale.y = 2.0f;
	tmpTransform.scale.z = 2.2f;
	tmpTransform.translate.y = -8.0f;
	worldTransform_.Initialize(tmpTransform);

	// 当たり判定を作成
	collider_ = std::make_unique<AABBCollider>();
	collider_->SetWorldPosition(transform.translate);
	Vector3 scale = transform.scale;
	scale.y = 30.0f;
	collider_->SetSize(scale);
	collider_->SetCollisionAttribute(kCollisionAttributeTerrain);
	collider_->SetCollisionMask(~kCollisionAttributeTerrain);

	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Wall);
	collider_->SetUserData(userData);

	// コールバック関数に登録する
	//collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
	//	this->OnCollisionEnter(result);
	//});

	// マテリアルの初期化
	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = 0;

#ifdef _DEBUG
// 値を登録する
	RegisterDebugParam();
#endif
	// 値を適応させる
	ApplyDebugParam();
}

void Wall::Update() {
#ifdef _DEBUG
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void Wall::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

}

AABB Wall::GetAABBData() {
	Vector3 halfSize = collider_->GetSize() * 0.5f;
	Vector3 pos = collider_->GetWorldPosition();
	return AABB{ pos - halfSize,pos + halfSize };
}

void Wall::RegisterDebugParam() {
	// マテリアル
	GameParamEditor::GetInstance()->AddItem(groupName_, "IceColor", iceMaterial_->materialData_->color);
	GameParamEditor::GetInstance()->AddItem(groupName_, "SpecularColor", specularColor);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimColor", rimColor);
	GameParamEditor::GetInstance()->AddItem(groupName_, "Shininess", iceMaterial_->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimIntensity", iceMaterial_->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimPower", iceMaterial_->materialData_->rimPower);
}

void Wall::ApplyDebugParam() {
	// マテリアル
	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "IceColor");
	specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "SpecularColor");
	rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "RimColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RimIntensity");
	iceMaterial_->materialData_->rimPower = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RimPower");

	iceMaterial_->materialData_->rimColor.x = rimColor.x;
	iceMaterial_->materialData_->rimColor.y = rimColor.y;
	iceMaterial_->materialData_->rimColor.z = rimColor.z;
	iceMaterial_->materialData_->specularColor.x = specularColor.x;
	iceMaterial_->materialData_->specularColor.y = specularColor.y;
	iceMaterial_->materialData_->specularColor.z = specularColor.z;
}