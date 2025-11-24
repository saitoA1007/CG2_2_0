#include"EnemyAttackManager.h"

EnemyAttackManager::~EnemyAttackManager() {
	IceFallsList_.clear();
}

void EnemyAttackManager::Update() {

	// 氷柱がデスフラグがたったら削除
	IceFallsList_.remove_if([](const std::unique_ptr<IceFall>& iceFall) {
		return iceFall->IsAlive() ? false : true;
	});

	// 氷柱の更新処理
	for (std::unique_ptr<IceFall>& iceFall : IceFallsList_) {
		iceFall->Update();
	}
}

void EnemyAttackManager::AddIceFall(const Vector3& pos) {
	std::unique_ptr<IceFall> tmpIceFall = std::make_unique<IceFall>();
	tmpIceFall->Initialize(pos);
	IceFallsList_.push_back(std::move(tmpIceFall));
}