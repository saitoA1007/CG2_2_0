#include"EnemyProjectileManager.h"

#include"Object/RockBullet.h"

EnemyProjectileManager::EnemyProjectileManager() {

}

EnemyProjectileManager::~EnemyProjectileManager() {

}

void EnemyProjectileManager::Initialize() {
    projectiles_.clear();
}

void EnemyProjectileManager::Update() {

    // 生存フラグが無効の時削除する
    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(),
            [](const std::unique_ptr<IProjectile>& projectile) {
                return !projectile->IsAlive();
            }),
        projectiles_.end()
    );

    // すべてを更新
    for (auto& projectile : projectiles_) {
        if (projectile->IsAlive()) {
            projectile->Update();
        }
    }
}

void EnemyProjectileManager::AddProjectile(const ProjectileSpwanPrams& prams) {
    auto projectile = CreateProjectile(prams);

    if (projectile) {
        projectiles_.push_back(std::move(projectile));
    }
}

std::unique_ptr<IProjectile> EnemyProjectileManager::CreateProjectile(const ProjectileSpwanPrams& prams) {
    std::unique_ptr<IProjectile> projectile;

    switch (prams.type) {
    case ProjectileType::Rock: {
        auto bullet = std::make_unique<RockBullet>();
        bullet->Initialize(prams.pos, prams.dir);
        projectile = std::move(bullet);
        break;
    }
    }
    return projectile;
}

std::vector<IProjectile*> EnemyProjectileManager::GetProjectilesByType(ProjectileType type) const {
    std::vector<IProjectile*> result;
    for (const auto& projectile : projectiles_) {
        if (projectile->IsAlive() && projectile->GetType() == type) {
            result.push_back(projectile.get());
        }
    }
    return result;
}