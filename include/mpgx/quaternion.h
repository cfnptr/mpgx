#pragma once
#include "mpgx/matrix.h"

struct Quaternion
{
	float x, y, z, w;
};

inline static struct Quaternion createQuaternion(
	float x,
	float y,
	float z,
	float w)
{
	struct Quaternion quaternion;
	quaternion.x = x;
	quaternion.y = y;
	quaternion.z = z;
	quaternion.w = w;
	return quaternion;
}

inline static struct Quaternion createEulerQuaternion(
	struct Vector3F eulerAngles)
{
	float s0 = sinf(eulerAngles.x * 0.5f);
	float s1 = sinf(eulerAngles.x * 0.5f);
	float s2 = sinf(eulerAngles.x * 0.5f);

	float c0 = cosf(eulerAngles.x * 0.5f);
	float c1 = cosf(eulerAngles.x * 0.5f);
	float c2 = cosf(eulerAngles.x * 0.5f);

	struct Quaternion quaternion;
	quaternion.x = s0 * c1 * c2 - c0 * s1 * s2;
	quaternion.y = c0 * s1 * c2 + s0 * c1 * s2;
	quaternion.z = c0 * c1 * s2 - s0 * s1 * c2;
	quaternion.w = c0 * c1 * c2 + s0 * s1 * s2;
	return quaternion;
}

inline static struct Matrix4F getQuaternionMatrixF4(
	struct Quaternion quaternion)
{
	float xx = quaternion.x * quaternion.x;
	float yy = quaternion.y * quaternion.y;
	float zz = quaternion.z * quaternion.z;
	float xz = quaternion.x * quaternion.z;
	float xy = quaternion.x * quaternion.y;
	float yz = quaternion.y * quaternion.z;
	float wx = quaternion.w * quaternion.x;
	float wy = quaternion.w * quaternion.y;
	float wz = quaternion.w * quaternion.z;

	struct Matrix4F matrix;
	matrix.m00 = 1.0f - 2.0f * (yy + zz);
	matrix.m01 = 2.0f * (xy - wz);
	matrix.m02 = 2.0f * (xz + wy);
	matrix.m03 = 0.0f;

	matrix.m10 = 2.0f * (xy + wz);
	matrix.m11 = 1.0f - 2.0f * (xx + zz);
	matrix.m12 = 2.0f * (yz - wx);
	matrix.m13 = 0.0f;

	matrix.m20 = 2.0f * (xz - wy);
	matrix.m21 = 2.0f * (yz + wx);
	matrix.m22 = 1.0f - 2.0f * (xx + yy);
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = 0.0f;
	matrix.m33 = 0.0f;
	return matrix;
}

inline static struct Quaternion dotQuaternion(
	struct Quaternion a,
	struct Quaternion b)
{
	a.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	a.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	a.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	a.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	return a;
}
inline static struct Quaternion normalizeQuaternion(
	struct Quaternion quaternion)
{
	float length = sqrtf(
		(quaternion.x * quaternion.x) +
		(quaternion.y * quaternion.y) +
		(quaternion.z * quaternion.z) +
		(quaternion.w * quaternion.w));

	if (length <= 0.0f)
	{
		quaternion.x = 0.0f;
		quaternion.y = 0.0f;
		quaternion.z = 0.0f;
		quaternion.w = 1.0f;
		return quaternion;
	}

	length = 1.0f / length;

	quaternion.x *= length;
	quaternion.y *= length;
	quaternion.z *= length;
	quaternion.w *= length;
	return quaternion;
}

inline static bool compareQuaternion(
	struct Quaternion a,
	struct Quaternion b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z &&
		a.w == b.w;
}
