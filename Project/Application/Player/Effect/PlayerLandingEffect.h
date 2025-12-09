#pragma once
#include<array>
#include<memory>
#include"WorldTransform.h"
#include"EngineSource/Math/Vector3.h"

namespace GameEngine { class Material; }

class PlayerLandingEffect {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// エミッタを起動 (着地エフェクト開始)
    /// </summary>
    /// <param name="pos">発生位置</param>
    void Emitter(const Vector3 &pos);

public:
    /// <summary>
    /// 親を設定
    /// </summary>
    /// <param name="parent"></param>
    void SetParent(GameEngine::WorldTransform* parent) {
        for (size_t i = 0; i < worldTransforms_.size(); ++i) {
            worldTransforms_[i].SetParent(parent);
        }
    }

    /// <summary>
    /// ワールド行列を取得
    /// </summary>
    /// <returns></returns>
    std::array<GameEngine::WorldTransform, 2>& GetWorldTransforms() { return worldTransforms_; }

    /// <summary>
    /// マテリアル取得
    /// </summary>
    std::array<std::unique_ptr<GameEngine::Material>, 2>& GetMaterials() { return materials_; }

    /// <summary>
    /// 有効状態取得
    /// </summary>
    bool IsActive() const { return isActive_; }

private:
    // ワールド行列
    std::array<GameEngine::WorldTransform,2> worldTransforms_;

    // マテリアル
    std::array<std::unique_ptr<GameEngine::Material>,2> materials_;

    // 有効フラグ
    bool isActive_ = false;

    // 内部タイマー
    float timer_ = 0.0f;

    // エフェクト継続時間
    float duration_ = 0.6f;
};
