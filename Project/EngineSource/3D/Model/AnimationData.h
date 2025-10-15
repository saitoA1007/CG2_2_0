#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Vector2.h"
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/MyMath.h"
#include"VertexData.h"

#include<vector>
#include<map>
#include<iostream>

template <typename tValue>
struct Keyframe {
	float time;  // キーフレームの時刻
	tValue value;  // キーフレームの値
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

struct NodeAnimation {
	std::vector<KeyframeVector3> translate;
	std::vector<KeyframeQuaternion> rotate;
	std::vector<KeyframeVector3> scale;
};

struct AnimationData {
	float duration;  // アニメーション全体の尺
	float timer = 0.0f;
	// NodeAnimationの集合体。Node名でひらけるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimations;
};