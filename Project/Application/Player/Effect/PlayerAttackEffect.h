#pragma once
#include"WorldTransform.h"
#include"Material.h"
#include"ParticleSystem/ParticleBehavior.h"

class PlayerAttackEffect {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(const uint32_t& texture);

    /// <summary>
    /// 更新処理
    /// </summary>s
    void Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix);

public:

    // エミッター
    void Emitter(const Vector3& pos);

    /// <summary>
    /// ワールド行列を取得
    /// </summary>
    /// <returns></returns>
    GameEngine::WorldTransform& GetWorldTransforms() { return worldTransform_; }

    /// <summary>
    /// マテリアルを取得
    /// </summary>
    /// <returns></returns>
    GameEngine::Material& GetMaterial() { return material_; }

    // 細かい粒子
    std::unique_ptr<GameEngine::ParticleBehavior> smallParticle_;

    // 終了フラグ
    bool IsActive()const { return isActive_; }

private:
    // ワールド行列
    GameEngine::WorldTransform worldTransform_;
    // マテリアル
    GameEngine::Material material_;

    // 終了フラグ
    bool isActive_ = false;

    // デバック
    std::string kGroupName_ = "PlayerAttackEffect";

    // 発生位置
    Vector3 emitPos_ = { 0.0f,0.0f,0.0f };

    // ヒットエフェクト
    float hitTimer_ = 0.0f;
    float hitMaxTime_ = 1.0f;
    float endScale_ = 4.0f;
    float endRotZ_ = 30.0f;

private:

    /// <summary>
    /// 値を登録する
    /// </summary>
    void RegisterBebugParam();

    /// <summary>
    /// 値を適応する
    /// </summary>
    void ApplyDebugParam();
};


