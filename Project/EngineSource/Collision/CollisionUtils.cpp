#include"CollisionUtils.h"

#include"MyMath.h"
#include<cmath>
#include<algorithm>

bool GameEngine::IsSpheresCollision(const Sphere& s1, const Sphere& s2) {
	// 半径の合計より短ければ衝突
	if (Length(Subtract(s1.center, s2.center)) < s1.radius + s2.radius) {
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsSpherePlaneCollision(const Sphere& sphere, const Plane& plane) {
	// 球の半径より短ければ衝突
	if (std::fabs(Dot(plane.normal, sphere.center) - plane.distance) <= sphere.radius) {
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsSegmentPlaneCollision(const Segment& segment, const Plane& plane) {
	// 垂直判定を行うために、法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);

	// 垂直の時は衝突していないのでfalseを返す
	if (dot == 0.0f) {
		return false;
	}

	// tを求める
	float t = (plane.distance - Dot(plane.normal, segment.origin)) / dot;

	if (t >= 0.0f && t <= 1.0f) {
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsSegmentTriangleCollision(const Triangle& triangle, const Segment& segment) {

	// 三角形の3つの頂点を使って平面を求める
	Plane plane;
	plane.normal = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(triangle.vertices[2], triangle.vertices[1]));
	plane.distance = Dot(plane.normal, triangle.vertices[0]);
	// 法線を正規化
	plane.normal = Normalize(plane.normal);
	// 垂直判定を行うために、法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);

	// tを求める
	float t = (plane.distance - Dot(plane.normal, segment.origin)) / dot;
	// 衝突点pを求める
	Vector3 p = Vector3(segment.origin) + Vector3(segment.diff.x * t, segment.diff.y * t, segment.diff.z * t);

	// 各辺を結んだベクトル
	Vector3 v01 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 v1p = p - triangle.vertices[1];
	Vector3 v12 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	Vector3 v2p = p - triangle.vertices[2];
	Vector3 v20 = Subtract(triangle.vertices[0], triangle.vertices[2]);
	Vector3 v0p = p - triangle.vertices[0];

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る
	Vector3 cross01 = Cross(v01, v1p);
	Vector3 cross12 = Cross(v12, v2p);
	Vector3 cross20 = Cross(v20, v0p);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突
	if (Dot(cross01, plane.normal) >= 0.0f &&
		Dot(cross12, plane.normal) >= 0.0f &&
		Dot(cross20, plane.normal) >= 0.0f) {
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsAABBCollision(const AABB& aabb1, const AABB& aabb2) {

	// 衝突判定
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) &&  // x軸
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) &&  // y軸
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) {  // z軸
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsAABBSphereCollision(const AABB& aabb, const Sphere& sphere) {

	// 最近接点を求める
	Vector3 closestPoint = {
		std::clamp(sphere.center.x,aabb.min.x,aabb.max.x),
		std::clamp(sphere.center.y,aabb.min.y,aabb.max.y),
		std::clamp(sphere.center.z,aabb.min.z,aabb.max.z)
	};

	// 距離が半径よりも小さければ衝突
	if (Length(closestPoint - sphere.center) <= sphere.radius) {
		return true;
	} else {
		return false;
	}
}

bool GameEngine::IsAABBSegmentCollision(const AABB& aabb, const Segment& segment) {

	// 各軸のnear,farを求める
	Vector3 tNear = Min({ (aabb.min.x - segment.origin.x) / segment.diff.x,(aabb.min.y - segment.origin.y) / segment.diff.y,(aabb.min.z - segment.origin.z) / segment.diff.z },
		{ (aabb.max.x - segment.origin.x) / segment.diff.x,(aabb.max.y - segment.origin.y) / segment.diff.y,(aabb.max.z - segment.origin.z) / segment.diff.z });
	Vector3 tFar = Max({ (aabb.min.x - segment.origin.x) / segment.diff.x,(aabb.min.y - segment.origin.y) / segment.diff.y,(aabb.min.z - segment.origin.z) / segment.diff.z },
		{ (aabb.max.x - segment.origin.x) / segment.diff.x,(aabb.max.y - segment.origin.y) / segment.diff.y,(aabb.max.z - segment.origin.z) / segment.diff.z });

	// AABBとの衝突点（貫通点）のtが小さい方
	float tMin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	// AABBとの衝突点（貫通点）のtが大きい方
	float tMax = std::min(std::min(tFar.x, tFar.y), tFar.z);

	// 範囲の外を出ていたらfalse
	if (tMin > 1.0f || tMax < 0.0f) {
		return false;
	}

	// 衝突した時
	if (tMin <= tMax) {
		return true;
	} else {
		return false;
	}
}