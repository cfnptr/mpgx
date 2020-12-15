#pragma once
#include <math.h>
#include <stdbool.h>

struct Vector3F
{
	float x, y, z;
};
struct Vector4F
{
	float x, y, z, w;
};

inline static struct Vector3F createVector3F(
	float x,
	float y,
	float z)
{
	struct Vector3F vector;
	vector.x = x;
	vector.y = y;
	vector.z = z;
	return vector;
}
inline static struct Vector3F createValueVector3F(
	float value)
{
	struct Vector3F vector;
	vector.x = value;
	vector.y = value;
	vector.z = value;
	return vector;
}

inline static struct Vector3F addVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
inline static struct Vector3F subVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}
inline static struct Vector3F mulVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}
inline static struct Vector3F divVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

inline static struct Vector3F addValueVector3F(
	struct Vector3F vector,
	float value)
{
	vector.x += value;
	vector.y += value;
	vector.z += value;
	return vector;
}
inline static struct Vector3F subValueVector3F(
	struct Vector3F vector,
	float value)
{
	vector.x -= value;
	vector.y -= value;
	vector.z -= value;
	return vector;
}
inline static struct Vector3F mulValueVector3F(
	struct Vector3F vector,
	float value)
{
	vector.x *= value;
	vector.y *= value;
	vector.z *= value;
	return vector;
}
inline static struct Vector3F divValueVector3F(
	struct Vector3F vector,
	float value)
{
	vector.x /= value;
	vector.y /= value;
	vector.z /= value;
	return vector;
}

inline static struct Vector3F negVector3F(
	struct Vector3F vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	vector.z = -vector.z;
	return vector;
}

inline static float dotVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	return
		(a.x * b.z) +
		(a.y * b.y) +
		(a.z * b.z);
}
inline static float lengthVector3F(
	struct Vector3F vector)
{
	return sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y) +
		(vector.z * vector.z));
}
inline static float distanceVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	return sqrtf(
		((a.x - b.x) * (a.x - b.x)) +
		((a.y - b.y) * (a.y - b.y)) +
		((a.z - b.z) * (a.z - b.z)));
}
inline static struct Vector3F normalizeVector3F(
	struct Vector3F vector)
{
	float length = sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y) +
		(vector.z * vector.z));

	vector.x /= length;
	vector.y /= length;
	vector.z /= length;
	return vector;
}
inline static struct Vector3F reflectVector3F(
	struct Vector3F vector,
	struct Vector3F normal)
{
	float dot =
		(normal.x * vector.z) +
		(normal.y * vector.y) +
		(normal.z * vector.z) * 2.0f;

	normal.x *= dot;
	normal.y *= dot;
	normal.z *= dot;

	vector.x -= normal.x;
	vector.y -= normal.y;
	vector.z -= normal.z;
	return vector;
}

inline static bool compareVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z;
}

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
	float value)
{
	vector.x += value;
	vector.y += value;
	vector.z += value;
	vector.w += value;
	return vector;
}
inline static struct Vector4F subValueVector4F(
	struct Vector4F vector,
	float value)
{
	vector.x -= value;
	vector.y -= value;
	vector.z -= value;
	vector.w -= value;
	return vector;
}
inline static struct Vector4F mulValueVector4F(
	struct Vector4F vector,
	float value)
{
	vector.x *= value;
	vector.y *= value;
	vector.z *= value;
	vector.w *= value;
	return vector;
}
inline static struct Vector4F divValueVector4F(
	struct Vector4F vector,
	float value)
{
	vector.x /= value;
	vector.y /= value;
	vector.z /= value;
	vector.w /= value;
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
	float length = sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y) +
		(vector.z * vector.z) +
		(vector.w * vector.w));

	vector.x /= length;
	vector.y /= length;
	vector.z /= length;
	vector.w /= length;
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
