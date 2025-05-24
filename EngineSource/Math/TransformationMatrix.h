#pragma once
#include"Matrix4x4.h"
#include"Vector4.h"
#include"Vector3.h"
#include<cstdint>

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct CameraForGPU {
	Vector3 worldPosition;
};