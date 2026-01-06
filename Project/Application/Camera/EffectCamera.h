#pragma once
#include<memory>
#include"Camera.h"

class EffectCamera {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// カメラデータ
	/// </summary>
	/// <returns></returns>
	GameEngine::Camera& GetCamera() const { return *camera_.get(); }

private:

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;

};