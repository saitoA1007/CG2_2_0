#pragma once
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include<array>

class Rope {
public:

	// ヒモの1頂点
	struct Node {
		Vector3 pos = {};      // 現在位置
		Vector3 prePos = {};   // 前フレームの位置。(一つ前のノードの位置ではないので注意！親子関係をしているわけではない)
		float radius = 0.0f;     // 半径
		Vector3 speed = {};    // 速度　いらないかも
		float mass = 0.0f;     // 質量
		bool locked = false;   // 固定するかを判定(外力の影響を受けない)
	};

public:

	~Rope();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model"></param>
	/// <param name="textureHandle"></param>
	void Initialize(GameEngine::Model* model,const uint32_t& textureHandle);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 先端を描画
	/// </summary>
	/// <param name="VPMatrix"></param>
	void DrawLine(const Matrix4x4& VPMatrix);

	/// <summary>
	/// 始点を描画
	/// </summary>
	/// <param name="VPMatrix"></param>
	void DrawSphere(const Matrix4x4& VPMatrix);

	void SetIsPressMouse(const bool& isPressMouse) { isPressMouse_ = isPressMouse; }

private:

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 矩形モデル
	GameEngine::Model* sphereModel_ = nullptr;

	static const uint32_t segmentCount = 32;

	// 線を描画する為のクラス
	GameEngine::PrimitiveRenderer* primitiveRenderer_;
	// 線の描画するためのメッシュデータ
	std::array<std::unique_ptr<GameEngine::LineMesh>, segmentCount> lineMeshs_;

	std::array<Node, segmentCount> points_;

	// 先頭
	Vector3 startPos_{};

	// デルタタイム
	const float dt = 1.0f / 60.0f;

	// 重力
	const float gravity = 9.8f;

	const float velocityThreshold = 0.01f;

	bool isPressMouse_ = false;
private:

	void SolveDistanceConstraints(std::array<Node, segmentCount>& rope, int iterations);
};