#pragma once
#include<list>
#include"Model.h"
#include"Wall.h"
#include"Effect/BreakWallParticle.h"

class StageManager {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const uint32_t& wallTexture);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="wallModel"></param>
	void Draw(GameEngine::Model* wallModel, ID3D12Resource* lightGroupResource);

	/// <summary>
	/// デバック用の更新処理
	/// </summary>
	void DebugUpdate();

public:

	/// <summary>
	/// 壁の要素を取得
	/// </summary>
	/// <returns></returns>
	std::vector<std::unique_ptr<Wall>>& GetWalls() { return walls_; }

	/// <summary>
	/// 生存している壁の要素を取得
	/// </summary>
	/// <returns></returns>
	std::vector<Wall*>& GetAliveWalls() { return aliveWalls_; }

	/// <summary>
	/// ステージの半径を取得
	/// </summary>
	/// <returns></returns>
	const float& GetRadius() const { return radius_; }

	/// <summary>
	/// 壊れた演出のリスト
	/// </summary>
	/// <returns></returns>
	std::list<std::unique_ptr<BreakWallParticle>>& GetBreakWallParticles() { return breakWallParticles_; }

private:

	// 生成位置
	Vector3 centerPosition_ = { 0.0f,1.0f,0.0f };

	// 半径
	float radius_ = 20.0f;

	// 辺の数
	uint32_t maxSideNumber_ = 6;

	// 復活するまでの時間
	float respawnTime_ = 3.0f;

	// 最大hp
	int32_t maxHp_ = 3;

	// 壁の奥行(1.0f = 1m)
	float wallDepth_ = 1.0f;
	// 壁の縦幅(1.0f = 1m)
	float wallHeight_ = 2.0f;

	// 壁の横幅の余剰分
	float offsetWallWidth_ = 2.0f;

	// 生成する
	bool isCreate_ = false;
	bool created_ = false;

	// 生成する壁
	std::vector<std::unique_ptr<Wall>> walls_;

	// 下の氷
	std::vector<std::unique_ptr<GameEngine::WorldTransform>> underWalls_;

	// 生存状態の壁のデータ
	std::vector<Wall*> aliveWalls_;

	// 壊れた演出
	std::list<std::unique_ptr<BreakWallParticle>> breakWallParticles_;

	uint32_t wallTextureHandle_ = 0;

	std::string kGroupName_ = "Wall";

	std::unique_ptr<IceRockMaterial> iceMaterial_;
	// デバック用
	Vector4 rimColor;
	Vector4 specularColor;

private:

	/// <summary>
	/// ステージを生成する処理
	/// </summary>
	void GenerateWalls();

	void AddBreakWallParticle(const Vector3& pos);

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};