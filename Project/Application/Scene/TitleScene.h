#pragma once
#include <d3d12.h>
#include <wrl.h>
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"WorldTransform.h"

class TitleScene : public BaseScene {
public:

	~TitleScene();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="input"></param>
	void Initialize(SceneContext* context) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// デバック時、処理して良いものを更新する
	/// </summary>
	void DebugUpdate() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const bool& isDebugView) override;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の名前を取得
	/// </summary>
	/// <returns></returns>
	std::string NextSceneName() override { return "Game"; }

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// 平面モデル
	GameEngine::Model* planeModel_ = nullptr;
	GameEngine::WorldTransform worldTransform_;

private:

	struct alignas(16) MaterialData {
		Vector4 color;
		Matrix4x4 uvTransform;
		uint32_t textureHandle;
		float timer;
		float padding[2];
	};

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	// マテリアルにデータを書き込む
	MaterialData* materialData_ = nullptr;

	ID3D12GraphicsCommandList* commandList_ = nullptr;

	GameEngine::SrvManager* srvManager_ = nullptr;
};