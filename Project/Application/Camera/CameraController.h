#pragma once
#include"Camera.h"
#include"InputCommand.h"
#include "Geometry.h"
#include <variant>
#include <vector>
#include "InPut.h"

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
	/// 更新処理 (マウス差分/右スティックで回転可能)
	/// </summary>
	/// <param name="inputCommand">入力コマンド</param>
	/// <param name="rawInput">低レイヤ入力</param>
	void Update(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput);

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

	/// <summary>
	/// 追加: FOVの目標値を外部から設定
	/// </summary>
	/// <param name="fov"></param>
	void SetDesiredFov(float fov) { desiredTargetFov_ = fov; }

private:

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	Vector3 position_ = { 0.0f,4.0f,-10.0f };

	// 距離
	static inline const float kDistance_ = 32.0f;

	// 回転の移動量(球面: θ=x(azimuth/yaw), φ=y(polar/pitch))
	Vector2 rotateMove_ = { 3.1f,1.0f };

	// ターゲット
    Target target_ = Vector3{ 0.0f,0.0f,0.0f };

	// 派生ターゲット情報(現在値)
	Vector3 targetPos_ = {0.0f,0.0f,0.0f};
    Vector3 targetLookAt_ = { 0.0f,0.0f,0.0f };
	Vector3 targetRotate_ = {0.0f,0.0f,0.0f};
	float   targetFov_ = 1.0f;

	// 目標(補間先)
	Vector3 desiredTargetPos_ = {0.0f,0.0f,0.0f};
    Vector3 desiredTargetLookAt_ = { 0.0f,0.0f,0.0f };
	Vector3 desiredTargetRotate_ = {0.0f,0.0f,0.0f};
	float   desiredTargetFov_ = 1.0f;

	// カメラ座標系
	CameraCoodinateType cameraCoordinateType_ = CameraCoodinateType::Spherical;

	// 入力感度
	float mouseRotateSensitivity_ = 0.1f; // マウス差分スケール
	float stickRotateSensitivity_ = 0.1f; // 右スティックスケール

private:
	void UpdateTargetVector3(const Vector3& v);
	void UpdateTargetLine(const Line& line);
	void UpdateTargetVector3Array(const std::vector<Vector3>& arr);
	void UpdateCartesian(GameEngine::InputCommand* inputCommand);
	void UpdateSpherical(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput);
	Matrix4x4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
};