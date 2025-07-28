#pragma once
#include"EngineSource/2D/Sprite.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"

class DrawTaskModel {
public:

	// 描画するモデル
	enum ModelMode {
		plane,
		sphere,
		UtahTeapot,
		StanfordBunny,
		Suzanne,
		MultiMesh,
		MultiMaterial
	};
	
public:

	~DrawTaskModel();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const uint32_t& uvCheckerGH,const uint32_t& whiteGH);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// デバック
	/// </summary>
	void DebugWindow();

	/// <summary>
	/// 2Dの描画処理
	/// </summary>
	void Draw2D();

	/// <summary>
	/// 3Dの描画処理
	/// </summary>
	/// <param name="VPMatrix"></param>
	void Draw3D(const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource);

public:

	/// <summary>
	/// 平面モデルを設定
	/// </summary>
	/// <param name="planeModel"></param>
	/// <param name="planeGH"></param>
	void SetPlane(GameEngine::Model* planeModel) { planeModel_ = planeModel; }

	/// <summary>
	/// 球モデルを設定
	/// </summary>
	/// <param name="sphereModel"></param>
	/// <param name="sphereGH"></param>
	void SetSphere(GameEngine::Model* sphereModel) { sphereModel_ = sphereModel; }

	/// <summary>
	/// ティーポッドモデルを設定
	/// </summary>
	/// <param name="UtahTeapotModel"></param>
	void SetUtahTeapot(GameEngine::Model* utahTeapotModel) {utahTeapotModel_ = utahTeapotModel;}

	/// <summary>
	/// ウサギモデルを設定
	/// </summary>
	/// <param name="StanfordBunnyModel"></param>
	void SetBunny(GameEngine::Model* bunnyModel) { bunnyModel_ = bunnyModel; }

	/// <summary>
	/// スザンヌモデルを設定
	/// </summary>
	/// <param name="suzanneModel"></param>
	void SetSuzanne(GameEngine::Model* suzanneModel) { suzanneModel_ = suzanneModel; }

	/// <summary>
	/// マルチメッシュモデルを設定
	/// </summary>
	/// <param name="multiMeshModel"></param>
	void SetMultiMesh(GameEngine::Model* multiMeshModel) { multiMeshModel_ = multiMeshModel; }

	/// <summary>
	/// マルチマテリアルモデルを設定
	/// </summary>
	/// <param name="multiMaterialModel"></param>
	void SetMultiMaterial(GameEngine::Model* multiMaterialModel) { multiMaterialModel_ = multiMaterialModel; }

private:

	// 描画モデル
	const char* kModelName_[7] = { "Plane","Sphere","UtahTeapot","StanfordBunny","Suzanne","MultiMesh","MultiMaterial"};
	int selectModelNum_ = 0;

	// スプライトの描画状態を管理
	bool isDrawSprite_ = true;

	// ライトの適応を管理
	bool isLightOn_ = true;

	// テクスチャ
	uint32_t uvCheckerGH_ = 0u;
	uint32_t whiteGH_ = 0u;

	// スプライト
	GameEngine::Sprite* sprite_;
	Transform uvSpriteTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Vector2 spritePos_ = { 0.0f,0.0f };

	// 行列
	GameEngine::WorldTransform worldTransform_;

	// 平面
	GameEngine::Model* planeModel_ = nullptr;
	Transform planeTransform_;

	// 球
	GameEngine::Model* sphereModel_ = nullptr;
	Transform sphereTransform_;

	// ティーポッド
	GameEngine::Model* utahTeapotModel_ = nullptr;
	Transform utahTeapotTransform_;

	// ウサギ
	GameEngine::Model* bunnyModel_ = nullptr;
	Transform bunnyTransform_;

	// スザンヌ
	GameEngine::Model* suzanneModel_ = nullptr;
	Transform suzanneTransform_;

	// マルチメッシュ
	GameEngine::Model* multiMeshModel_ = nullptr;
	Transform multiMeshTransform_;

	// マルチマテリアル
	GameEngine::Model* multiMaterialModel_ = nullptr;
	Transform multiMaterialTransform_;
};