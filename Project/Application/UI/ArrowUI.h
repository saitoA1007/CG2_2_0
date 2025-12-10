#pragma once
#include"WorldTransform.h"
#include"Material.h"

class ArrowUI {
public:

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="maxHp"></param>
    void Initialize(const uint32_t& texture);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update(const Matrix4x4& cameraMatrix);

    GameEngine::WorldTransform* GetWorldTransform() { return &worldTransform_; }

    GameEngine::Material& GetMaterial() { return material_; }

    bool IsActive() { return isActive_; }

    void SetIsActive(const bool& isActice){
        isActive_ = isActice;
    }

private:

    GameEngine::WorldTransform worldTransform_;

    GameEngine::Material material_;

    bool isActive_ = false;

    float timer_ = 0.0f;

private:
    /// <summary>
    /// 演出の更新処理（個数式では未使用）
    /// </summary>
    void EffectUpdate() {}
};
