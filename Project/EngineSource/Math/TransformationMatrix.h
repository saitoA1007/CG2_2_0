#pragma once
#include"Matrix4x4.h"
#include"Vector4.h"
#include"Vector3.h"

struct alignas(16) TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 worldInverseTranspose;
};

struct alignas(16) ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct alignas(16) CameraForGPU {
	Vector3 worldPosition;
	float pad;
};