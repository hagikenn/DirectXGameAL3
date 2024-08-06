#include "Vector3.h"
#include <assert.h>
#include "Matrix4x4.h"



Vector3& operator+=(Vector3& lhv, const Vector3& rhv);
float EaseInOut(float x1, float x2, float t);
float Lerp(float x1, float x2, float t);
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
const Vector3 operator+(const Vector3& v1, const Vector3& v2);

Vector3& operator*=(Vector3& v, float s);
const Vector3 operator*(const Vector3& v, float s);

Matrix4x4 MakeRotateZMatrix(float radian);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
