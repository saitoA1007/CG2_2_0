#pragma once
#include"Math/Vector3.h"
#include"Math/Matrix4x4.h"
#include"InPut.h"

class DebugCamera {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(int width, int height);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(Input* input);

	Matrix4x4 GetVPMatrix();
	
private:
	// 拡縮
	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	// X,Y,Z軸回りのローカル座標角
	Vector3 rotate_ = { 0.0f,0.0f,0.0f };
	// ローカル座標
	Vector3 translate_ = { 0.0f,0.0f,-10.0f };
	// ビュー行列
	Matrix4x4 viewMatrix_;
	// 射影行列
	Matrix4x4 projectionMatrix_;

	// 累積回転行列
	Matrix4x4 rotateMatrix_;

	Vector3 target = { 0.0f, 0.0f, 0.0f };
	float distance = 10.0f;
};