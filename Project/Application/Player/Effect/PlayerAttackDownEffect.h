#pragma once
#include<array>
#include<memory>
#include"WorldTransform.h"

namespace GameEngine { class Material; }

class PlayerAttackDownEffect {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

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
    std::array<GameEngine::WorldTransform, 3>& GetWorldTransforms() { return worldTransforms_; }

    // マテリアルの取得
    GameEngine::Material* GetMaterial() const { return material_.get(); }

    // 透明度を設定
    void SetAlpha(float a);

    // ダメージ量に応じて色を変化
    void SetColorFromAttackPower(float minPower, float maxPower, float currentPower);

private:
    // ワールド行列
    std::array<GameEngine::WorldTransform,3> worldTransforms_;

    // 回転速度
    float rotateSpeed_ = 50.0f;

    // 有効フラグ
    bool isActive_ = false;

    // マテリアル
    std::unique_ptr<GameEngine::Material> material_;
};
