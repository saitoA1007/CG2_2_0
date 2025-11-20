#pragma once
#include"Camera.h"
#include"InputCommand.h"
#include "Geometry.h"
#include <variant>
#include <vector>

class CameraController {
public:
    enum class CameraCoodinateType {
        Cartesian,
		Spherical,
    };
    using Target = std::variant<Vector3, Line, std::vector<Vector3>>;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="inputCommand"></param>
	/// <param name="targetPos"></param>
	void Update(GameEngine::InputCommand* inputCommand);

	/// <summary>
    /// ターゲットの設定
    /// </summary>
	/// <param name="target"></param>
    void SetTarget(const Target &target) { target_ = target; }

	/// <summary>
	/// カメラの座標系種類を設定
	/// </summary>
	/// <param name="type"></param>
	void SetCameraCoordinateType(CameraCoodinateType type) { cameraCoordinateType_ = type; }

	/// <summary>
	/// カメラデータ
	/// </summary>
	/// <returns></returns>
	GameEngine::Camera& GetCamera() const { return *camera_.get(); }

private:
	GameEngine::InputCommand *inputCommand_ = nullptr;

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	Vector3 position_ = { 0.0f,4.0f,-10.0f };

	// 距離
	static inline const float kDistance_ = 40.0f;

	// 回転の移動量(球面: θ=x, φ=y)
	Vector2 rotateMove_ = { 3.1f,1.0f };

	// ターゲット
    Target target_ = Vector3{ 0.0f,0.0f,0.0f };

	// 派生したターゲット情報(現在値)
	Vector3 targetPos_ = {0.0f,0.0f,0.0f};
	Vector3 targetRotate_ = {0.0f,0.0f,0.0f};
	float   targetFov_ = 60.0f; // 視野(度)

	// 目標(理想)のターゲット情報(補間先)
	Vector3 desiredTargetPos_ = {0.0f,0.0f,0.0f};
	Vector3 desiredTargetRotate_ = {0.0f,0.0f,0.0f};
	float   desiredTargetFov_ = 60.0f;

	// カメラ座標系
	CameraCoodinateType cameraCoordinateType_ = CameraCoodinateType::Spherical;

private:

	/// <summary>
	/// TargetがVector3の時の更新
	/// </summary>
	void UpdateTargetVector3(const Vector3& v);
	/// <summary>
	/// TargetがLineの時の更新
	/// </summary>
	void UpdateTargetLine(const Line& line);
	/// <summary>
	/// TargetがVector3配列の時の更新(平均位置)
	/// </summary>
	void UpdateTargetVector3Array(const std::vector<Vector3>& arr);

	/// <summary>
	/// カメラ座標系: デカルト(直接position_を入力で移動)
	/// </summary>
	void UpdateCartesian(GameEngine::InputCommand* inputCommand);
	/// <summary>
	/// カメラ座標系: 球面(既存の回転量から算出)
	/// </summary>
	void UpdateSpherical(GameEngine::InputCommand* inputCommand);

	/// <summary>
	/// カメラをターゲットの方向に向かせる
	/// </summary>
	/// <param name="eye">カメラの位置</param>
	/// <param name="center">ターゲットの位置</param>
	/// <param name="up">向き</param>
	/// <returns></returns>
	Matrix4x4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
};