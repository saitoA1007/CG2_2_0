#include"DebugCamera.h"
#include"EngineSource/Math/MyMath.h"
#include"EngineSource/Common/CreateBufferResource.h"
#include"EngineSource/2D/ImGuiManager.h"

using namespace GameEngine;

void DebugCamera::Initialize(const Vector3& translate,int width, int height, ID3D12Device* device) {
	translate_ = translate;
	viewMatrix_ = InverseMatrix(MakeAffineMatrix(scale_, rotate_, translate_));
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 200.0f);
	rotateMatrix_ = LookAt(translate_, targetPos_, { 0.0f,1.0f,0.0f });

	// カメラリソースを作成
	cameraResource_ = CreateBufferResource(device, sizeof(CameraForGPU));
	// データを書き込む
	// 書き込むためのアドレスを取得
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
	// 単位行列を書き込んでおく
	cameraForGPU_->worldPosition = translate_;

	// 球面座標系で移動
	translate_.x = targetPos_.x + distance_ * std::sinf(mouseMove_.y) * std::sinf(mouseMove_.x);
	translate_.y = targetPos_.y + distance_ * std::cosf(mouseMove_.y);
	translate_.z = targetPos_.z + distance_ * std::sinf(mouseMove_.y) * std::cosf(mouseMove_.x);
	// 回転行列に変換
	rotateMatrix_ = LookAt(translate_, targetPos_, { 0.0f,1.0f,0.0f });
	// ワールド行列
	worldMatrix_ = rotateMatrix_;
	worldMatrix_.m[3][0] = translate_.x;
	worldMatrix_.m[3][1] = translate_.y;
	worldMatrix_.m[3][2] = translate_.z;

	//worldMatrix_ = MakeTranslateMatrix(translate_);
	cameraForGPU_->worldPosition = GetWorldPosition();
	// カメラの変更した内容を適用する処理
	viewMatrix_ = InverseMatrix(worldMatrix_);
}

void DebugCamera::Update(Input* input) {

	// 中クリックで移動
	if (input->PushMouse(2)) {

		// ターゲットの移動量
		Vector3 targetMove{0.0f,0.0f,0.0f};

		// X軸の移動
		if (input->GetMouseDelta().x > 0.0f) { targetMove.x = -1.0f; }
		if (input->GetMouseDelta().x < 0.0f) { targetMove.x = 1.0f; }
		// Y軸の移動
		if (input->GetMouseDelta().y < 0.0f) { targetMove.y = -1.0f; }
		if (input->GetMouseDelta().y > 0.0f) { targetMove.y = 1.0f; }

		// カメラの向き
		Vector3 forward = Normalize(targetPos_ - translate_);  
		// 上方向
		Vector3 up = { 0.0f, 1.0f, 0.0f };
		// 横方向
		Vector3 right = Normalize(Cross(up, forward));
		// カメラから見てx,y軸に移動量を求める
		Vector3 moveVec = right * targetMove.x + up * targetMove.y;
		// ターゲットに加算
		targetPos_ += moveVec * kTargetSpeed;
	} else {

		// ホイールで距離を調整する
		distance_ -= input->GetWheel() * 0.05f;
		distance_ = std::clamp(distance_, 2.0f, 100.0f);

		// 右クリックで回転する処理
		if (input->PushMouse(1)) {
			mouseMove_ += input->GetMouseDelta() * 0.05f;
		}
	}
	
	// 球面座標系で移動
	translate_.x = targetPos_.x + distance_ * std::sinf(mouseMove_.y) * std::sinf(mouseMove_.x);
	translate_.y = targetPos_.y + distance_ * std::cosf(mouseMove_.y);
	translate_.z = targetPos_.z + distance_ * std::sinf(mouseMove_.y) * std::cosf(mouseMove_.x);
	
	// 回転行列に変換
	rotateMatrix_ = LookAt(translate_, targetPos_, {0.0f,1.0f,0.0f});

	// ワールド行列
	worldMatrix_ = rotateMatrix_;
	worldMatrix_.m[3][0] = translate_.x;
	worldMatrix_.m[3][1] = translate_.y;
	worldMatrix_.m[3][2] = translate_.z;

	cameraForGPU_->worldPosition = GetWorldPosition();
	// カメラの変更した内容を適用する処理
	viewMatrix_ = InverseMatrix(worldMatrix_);

#ifdef _DEBUG

	ImGui::Begin("DebugCamera");
	ImGui::DragFloat2("mouseDelta", &mouseMove_.x, 0.1f);
	ImGui::Text("x:%f,y:%f", input->GetMouseDelta().x, input->GetMouseDelta().y);
	ImGui::DragFloat3("CameraPos", &translate_.x,0.01f);
	ImGui::DragFloat3("CaeraRotate", &rotate_.x, 0.01f);

	if (ImGui::Button("ResetTargetPosition")) {
		targetPos_ = { 0.0f,0.0f,0.0f };
	}

	ImGui::End();
#endif

}

Matrix4x4 DebugCamera::GetVPMatrix() {
	return Multiply(viewMatrix_, projectionMatrix_);
}

Matrix4x4 DebugCamera::GetRotateMatrix() {
	return rotateMatrix_;
}

Vector3 DebugCamera::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldMatrix_.m[3][0];
	worldPos.y = worldMatrix_.m[3][1];
	worldPos.z = worldMatrix_.m[3][2];
	return worldPos;
}

Matrix4x4 DebugCamera::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
	Vector3 f = Normalize(center - eye); // 前方向ベクトル
	Vector3 s = Normalize(Cross(up,f)); // 右方向ベクトル
	Vector3 u = Cross(f,s); // 上方向ベクトル

	Matrix4x4 result = { {
		{ s.x,  s.y, s.z, 0 },
		{ u.x,  u.y, u.z, 0 },
		{ f.x,  f.y, f.z, 0 },
		{ 0.0f, 0.0f, 0.0f, 1}
	} };
	return result;
}