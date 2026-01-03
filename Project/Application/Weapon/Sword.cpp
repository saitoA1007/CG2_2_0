#include"Sword.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"MyMath.h"
using namespace GameEngine;

void Sword::Initialize() {

	// ワールド行列を初期化
	worldTransform_.Initialize({{1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}});

	// 当たり判定の位置を設定
	colliderPos_ = { 0.0f,5.0f,0.0f };
	colliderWorldMatrix_ = MakeTranslateMatrix(colliderPos_);

	// 当たり判定を初期化
	std::unique_ptr<SphereCollider> collider = std::make_unique<SphereCollider>();
	collider->SetWorldPosition(worldTransform_.GetWorldPosition());
	collider->SetRadius(1.0f);
	collider->SetCollisionAttribute(kCollisionAttributePlayer);
	collider->SetCollisionMask(~kCollisionAttributePlayer);
	// 当たり判定システムにプレイヤーのデータを設定する
	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Weapon);
	//userData.object = this;
	collider->SetUserData(userData);
	// コールバック関数を登録する
	//collider->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
	//	this->OnCollisionEnter(result);
	//	});
	
	// 当たり判定を登録
	colliders_.push_back(std::move(collider));
}

void Sword::Update() {

	// ワールド行列を更新
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	Matrix4x4 worldMatrix = colliderWorldMatrix_ * worldTransform_.GetWorldMatrix();
	Vector3 worldPos = Vector3(worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2]);
	colliders_[0]->SetWorldPosition(worldPos);
}