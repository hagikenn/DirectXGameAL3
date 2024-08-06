#include "MathUtilityForText.h"
#include <cmath>
#include <numbers>




Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}



float EaseInOut(float x1, float x2, float t) {
	float easedT = -(std::cosf(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
	return Lerp(x1, x2, easedT);
}

float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) { return Vector3(Lerp(v1.x, v2.x, t), Lerp(v1.y, v2.y, t), Lerp(v1.z, v2.z, t)); }

const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp += v2;
}

Vector3& operator*=(Vector3& v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

const Vector3 operator*(const Vector3& v, float s) {
	Vector3 temp(v);
	return temp *= s;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result;
	float c = cos(radian);
	float s = sin(radian);

	result.mat[0][0] = c;
	result.mat[0][1] = s;
	result.mat[0][2] = 0.0f;
	result.mat[0][3] = 0.0f;

	result.mat[1][0] = -s;
	result.mat[1][1] = c;
	result.mat[1][2] = 0.0f;
	result.mat[1][3] = 0.0f;

	result.mat[2][0] = 0.0f;
	result.mat[2][1] = 0.0f;
	result.mat[2][2] = 1.0f;
	result.mat[2][3] = 0.0f;

	result.mat[3][0] = 0.0f;
	result.mat[3][1] = 0.0f;
	result.mat[3][2] = 0.0f;
	result.mat[3][3] = 1.0f;

	return result;
}

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.mat[0][0] + vector.y * matrix.mat[1][0] + vector.z * matrix.mat[2][0] + 1.0f * matrix.mat[3][0];
	result.y = vector.x * matrix.mat[0][1] + vector.y * matrix.mat[1][1] + vector.z * matrix.mat[2][1] + 1.0f * matrix.mat[3][1];
	result.z = vector.x * matrix.mat[0][2] + vector.y * matrix.mat[1][2] + vector.z * matrix.mat[2][2] + 1.0f * matrix.mat[3][2];
	float w = vector.x * matrix.mat[0][3] + vector.y * matrix.mat[1][3] + vector.z * matrix.mat[2][3] + 1.0f * matrix.mat[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}