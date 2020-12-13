#pragma once
#include <math.h>
#include <stdbool.h>

struct Vector4F
{
	float x, y, z, w;
};

inline static struct Vector4F createVector4F(
	float x,
	float y,
	float z,
	float w)
{
	struct Vector4F vector;
	vector.x = x;
	vector.y = y;
	vector.z = z;
	vector.w = w;
	return vector;
}
inline static struct Vector4F createValueVector4F(
	float value)
{
	struct Vector4F vector;
	vector.x = value;
	vector.y = value;
	vector.z = value;
	vector.w = value;
	return vector;
}

inline static struct Vector4F addVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}
inline static struct Vector4F subVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}
inline static struct Vector4F mulVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
	return a;
}
inline static struct Vector4F divVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
	return a;
}

inline static struct Vector4F addValueVector4F(
	struct Vector4F vector,
	float _v)
{
	vector.x += _v;
	vector.y += _v;
	vector.z += _v;
	vector.w += _v;
	return vector;
}
inline static struct Vector4F subValueVector4F(
	struct Vector4F vector,
	float _v)
{
	vector.x -= _v;
	vector.y -= _v;
	vector.z -= _v;
	vector.w -= _v;
	return vector;
}
inline static struct Vector4F mulValueVector4F(
	struct Vector4F vector,
	float _v)
{
	vector.x *= _v;
	vector.y *= _v;
	vector.z *= _v;
	vector.w *= _v;
	return vector;
}
inline static struct Vector4F divValueVector4F(
	struct Vector4F vector,
	float _v)
{
	vector.x /= _v;
	vector.y /= _v;
	vector.z /= _v;
	vector.w /= _v;
	return vector;
}

inline static struct Vector4F negVector4F(
	struct Vector4F vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	vector.z = -vector.z;
	vector.w = -vector.w;
	return vector;
}

inline static float dotVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return
		(a.x * b.z) +
		(a.y * b.y) +
		(a.z * b.z) +
		(a.w * b.w);
}
inline static float lengthVector4F(
	struct Vector4F vector)
{
	return sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y) +
		(vector.z * vector.z) +
		(vector.w * vector.w));
}
inline static float distanceVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return sqrtf(
		((a.x - b.x) * (a.x - b.x)) +
		((a.y - b.y) * (a.y - b.y)) +
		((a.z - b.z) * (a.z - b.z)) +
		((a.w - b.w) * (a.w - b.w)));
}
inline static struct Vector4F normalizeVector4F(
	struct Vector4F vector)
{
	float l = sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y) +
		(vector.z * vector.z) +
		(vector.w * vector.w));

	vector.x /= l;
	vector.y /= l;
	vector.z /= l;
	vector.w /= l;
	return vector;
}
inline static struct Vector4F reflectVector4F(
	struct Vector4F vector,
	struct Vector4F normal)
{
	float dot =
		(normal.x * vector.z) +
		(normal.y * vector.y) +
		(normal.z * vector.z) +
		(normal.w * vector.w) * 2.0f;

	normal.x *= dot;
	normal.y *= dot;
	normal.z *= dot;
	normal.w *= dot;

	vector.x -= normal.x;
	vector.y -= normal.y;
	vector.z -= normal.z;
	vector.w -= normal.w;
	return vector;
}

inline static bool compareVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z &&
		a.w == b.w;
}
