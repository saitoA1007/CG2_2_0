#pragma once

struct Vector3 {
	float x, y, z;

	Vector3 operator+(const Vector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
	Vector3 operator-(const Vector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
	Vector3 operator*(const Vector3& other) const { return { x * other.x, y * other.y, z * other.z }; }
	Vector3 operator/(const Vector3& other) const { return { x / other.x, y / other.y, z / other.z }; }
	Vector3 operator+=(const Vector3& other) { return { x += other.x, y += other.y, z += other.z }; }
	Vector3 operator-=(const Vector3& other) { return { x -= other.x, y -= other.y, z -= other.z }; }
	Vector3 operator*=(const Vector3& other) { return { x *= other.x, y *= other.y, z *= other.z }; }
	Vector3 operator/=(const Vector3& other) { return { x /= other.x, y /= other.y, z /= other.z }; }
	Vector3 operator+(const float& other) const { return { x + other, y + other, z + other }; }
	Vector3 operator-(const float& other) const { return { x - other, y - other, z - other }; }
	Vector3 operator*(const float& other) const { return { x * other, y * other, z * other }; }
	Vector3 operator/(const float& other) const { return { x / other, y / other, z / other }; }
	Vector3 operator/=(const float& other) { return { x /= other, y /= other, z /= other }; }
	
	friend Vector3 operator*(float other, const Vector3& v) { return { v.x * other, v.y * other, v.z * other }; }
	Vector3 operator*=(const float& other) { x *= other; y *= other; z *= other; return *this; }
	Vector3 operator-() const { return { -x, -y, -z }; }
};