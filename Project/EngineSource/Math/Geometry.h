#pragma once
#include"Vector3.h"

struct Plane {
	Vector3 normal; // 法線
	float distance; // 距離
};

struct Line {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Ray {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Segment {
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};

struct Triangle {
	Vector3 vertices[3]; // 頂点
};

struct Sphere {
	Vector3 center; // 中心
	float radius;   // 半径
};

struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点
};

struct Box {
	Vector3 vertices[8]; // 頂点
};