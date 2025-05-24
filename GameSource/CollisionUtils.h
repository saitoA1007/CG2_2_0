#pragma once
#include"EngineSource/Math/Geometry.h"

// 球同士の衝突判定
bool IsSpheresCollision(const Sphere& s1, const Sphere& s2);

// 平面と球の衝突判定
bool IsSpherePlaneCollision(const Sphere& sphere, const Plane& plane);

// 線と平面の衝突判定
bool IsSegmentPlaneCollision(const Segment& segment, const Plane& plane);

// 線と三角形の衝突判定
bool IsSegmentTriangleCollision(const Triangle& triangle, const Segment& segment);

// aabb同士の衝突判定
bool IsAABBCollision(const AABB& aabb1, const AABB& aabb2);

// aabbの範囲に入っているかをの判定
bool IsAABBPosintCollision(const AABB& aabb, const Vector3& point);

// aabbと球の衝突判定
bool IsAABBSphereCollision(const AABB& aabb, const Sphere& sphere);