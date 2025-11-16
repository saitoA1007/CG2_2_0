#include"Wall.h"

using namespace GameEngine;

void Wall::Initialilze(const Transform& transform, float respawnTime, uint32_t maxHp) {

	// 復活までの時間を取得
	respawnTime_ = respawnTime;

	// 最大hpを取得
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);
}

void Wall::Update() {

	// 生存フラグがtrueなら早期リターン
	if (isAlive_) {return;}
}