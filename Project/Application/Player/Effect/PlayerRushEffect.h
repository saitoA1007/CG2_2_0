#pragma once
#include<array>
#include"WorldTransform.h"

class PlayerRushEffect {
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

private:
    // ワールド行列
    std::array<GameEngine::WorldTransform,3> worldTransforms_;

    // 回転速度
    float rotateSpeed_ = 50.0f;

    // 有効フラグ
    bool isActive_ = false;
};
