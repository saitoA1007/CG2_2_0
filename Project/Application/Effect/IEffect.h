#pragma once
#include"Vector3.h"

enum class EffectType {
	Particle, // パーティクル演出
	Animation, // アニメーション、演出
	Sound, // 音声演出
	ShakeCamera, // カメラ演出

	MaxCount // 数
};

class IEffect {
public:
    virtual ~IEffect() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    virtual void Initialize(const Vector3& position) = 0;

    /// <summary>
    /// 更新処理
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 演出が終了したか
    /// </summary>
    virtual bool IsFinished() const = 0;

    /// <summary>
    /// 演出を停止
    /// </summary>
    virtual void Stop() = 0;

    /// <summary>
    /// 演出の種類を取得
    /// </summary>
    virtual EffectType GetType() const = 0;

    /// <summary>
    /// ワールド座標を取得
    /// </summary>
    virtual Vector3 GetPosition() const = 0;
};