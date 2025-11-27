#pragma once
#include"Camera.h"
#include"InputCommand.h"
#include "Geometry.h"
#include <variant>
#include <vector>
#include "InPut.h"

template <typename T>
struct AnimationKeyframe {
	float time; // キーフレームの時間
	T value;    // キーフレームの値
    std::function<T(const T &, const T &, float)> interpFunc; // 補間関数
};

template<typename T>
struct AnimationState {
	std::vector<AnimationKeyframe<T>> keyframes; // キーフレーム配列
    size_t currentIndex = 0;	// 現在のキーフレームインデックス
};

class CameraController {
public:
    enum class CameraCoodinateType {
        Cartesian,
		Spherical,
    };
    enum class ShakeOrigin {
		TargetPosition,
		CameraPosition,
        TargetAndCameraPosition,
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
	/// FOVの目標値を外部から設定
	/// </summary>
	/// <param name="fov"></param>
	void SetDesiredFov(float fov) { desiredTargetFov_ = fov; }

    /// <summary>
    /// カメラシェイク開始
	/// </summary>
	/// <param name="power">強度</param>
	/// <param name="time">時間</param>
	/// <param name="origin">揺らす対象</param>
	void StartCameraShake(float power, float time, ShakeOrigin origin = ShakeOrigin::CameraPosition,
        bool enableX = true, bool enableY = true, bool enableZ = true) {
		cameraShakePower_ = power;
		cameraShakeMaxTime_ = time;
		cameraShakeElapsedTime_ = 0.0f;
        shakeOrigin_ = origin;
        enableShakeX_ = enableX;
        enableShakeY_ = enableY;
        enableShakeZ_ = enableZ;
    }

	/// <summary>
	/// 自動回転の全体倍率設定
	/// </summary>
	void SetAutoRotateGain(float g) { autoRotateOverallGain_ = g; }

    /// <summary>
    /// カメラアニメーション用キーフレームの設定
    /// </summary>
    /// <param name="positionKeyframes">位置キーフレーム配列</param>
    /// <param name="rotateKeyframes">回転キーフレーム配列</param>
    /// <param name="lookAtKeyframes">注視点キーフレーム配列</param>
	/// <param name="fovKeyframes">FOVキーフレーム配列</param>
	void SetAnimationKeyframes(
		const std::vector<AnimationKeyframe<Vector3>>& positionKeyframes,
        const std::vector<AnimationKeyframe<Vector3>> &rotateKeyframes,
		const std::vector<AnimationKeyframe<Vector3>>& lookAtKeyframes,
		const std::vector<AnimationKeyframe<float>>& fovKeyframes) {
		positionAnimationState_.keyframes = positionKeyframes;
        rotateAnimationState_.keyframes = rotateKeyframes;
		lookAtAnimationState_.keyframes = lookAtKeyframes;
        fovAnimationState_.keyframes = fovKeyframes;
    }

	/// <summary>
	/// カメラアニメーション再生開始
	/// </summary>
    /// <param name="playSpeed">再生速度</param>
    void PlayAnimation(float playSpeed = 1.0f) {
		if (positionAnimationState_.keyframes.empty() &&
            rotateAnimationState_.keyframes.empty() &&
			lookAtAnimationState_.keyframes.empty() &&
			fovAnimationState_.keyframes.empty()) {
			return; // キーフレームが無ければ再生しない
        }
		animationTime_ = 0.0f;
		animationPlaySpeed_ = playSpeed;
		isAnimationPlaying_ = true;
    }

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

	// カメラシェイク強度
    float cameraShakePower_ = 0.0f;
    // カメラシェイク最大時間
	float cameraShakeMaxTime_ = 0.0f;
    // カメラシェイク経過時間
    float cameraShakeElapsedTime_ = 0.0f;
    // シェイクの基準
    ShakeOrigin shakeOrigin_ = ShakeOrigin::CameraPosition;
	// X軸シェイク有効フラグ
    bool enableShakeX_ = true;
	// Y軸シェイク有効フラグ
    bool enableShakeY_ = true;
	// Z軸シェイク有効フラグ
    bool enableShakeZ_ = true;
	// 自動回転用: 前フレームのターゲット位置
	Vector3 prevTargetPos_ = {0.0f,0.0f,0.0f};
	// 自動回転感度(左右移動→ヨー回転)
	float autoRotateYawSensitivity_ = 0.2f; // 大きいほど強く追従
	// ターゲット接近時の回転倍率スケール
	float autoRotateApproachScale_ = 1.0f; // 接近速度 * scale を乗算
	// 自動回転の全体倍率
	float autoRotateOverallGain_ = 0.4f;

	//--------- アニメーション用 ---------//

    // カメラのアニメーション状態
    AnimationState<Vector3> positionAnimationState_;
    AnimationState<Vector3> rotateAnimationState_;
    AnimationState<Vector3> lookAtAnimationState_;
    AnimationState<float> fovAnimationState_;
    // アニメーション時間
    float animationTime_ = 0.0f;
	// アニメーション再生速度
    float animationPlaySpeed_ = 1.0f;
    // 再生中フラグ
    bool isAnimationPlaying_ = false;

private:
	void UpdateTargetVector3(const Vector3& v);
	void UpdateTargetLine(const Line& line);
	void UpdateTargetVector3Array(const std::vector<Vector3>& arr);
	void UpdateCartesian(GameEngine::InputCommand* inputCommand);
	void UpdateSpherical(GameEngine::InputCommand* inputCommand, GameEngine::Input* rawInput);
	void ApplyAutoRotate(const Vector3& eye, const Vector3& center);
	Matrix4x4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

	// アニメーション更新
    void UpdateAnimation();
};