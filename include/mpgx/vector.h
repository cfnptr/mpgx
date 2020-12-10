#pragma once
#include <math.h>
#include <stdbool.h>

struct Vector4F
{
	float x, y, z, w;
};

inline static struct Vector4F createVector4F(
	float x, float y, float z, float w)
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
	return createVector4F(
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w);
}
inline static struct Vector4F subVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return createVector4F(
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w);
}
inline static struct Vector4F mulVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return createVector4F(
		a.x * b.x,
		a.y * b.y,
		a.z * b.z,
		a.w * b.w);
}
inline static struct Vector4F divVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return createVector4F(
		a.x / b.x,
		a.y / b.y,
		a.z / b.z,
		a.w / b.w);
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
	struct Vector4F v = mulVector4F(a, b);
	return v.x + v.y + v.z + v.w;
}
inline static float lengthVector4F(
	struct Vector4F vector)
{
	return sqrtf(dotVector4F(vector, vector));
}
inline static float distanceVector4F(
	struct Vector4F a,
	struct Vector4F b)
{
	return lengthVector4F(subVector4F(a, b));
}

inline static struct Vector4F normalizeVector4F(
	struct Vector4F vector)
{
	return divValueVector4F(vector, lengthVector4F(vector));
}
inline static struct Vector4F faceForwardVector4F(
	struct Vector4F normal,
	struct Vector4F vector,
	struct Vector4F nRef)
{
	float dot = dotVector4F(nRef, vector);

	if (dot < 0)
		return normal;
	else
		return negVector4F(normal);
}
inline static struct Vector4F reflectVector4F(
	struct Vector4F vector,
	struct Vector4F normal)
{
	return subVector4F(vector,
		mulValueVector4F(normal,
		dotVector4F(normal, vector) * 2.0f));
}
inline static struct Vector4F refractVector4F(
	struct Vector4F vector,
	struct Vector4F normal,
	float eta)
{
	float d = 1.0f - eta * eta *
		(1.0f - dotVector4F(normal, vector) *
		dotVector4F(normal, vector));

	if (d < 0.0f)
		return createValueVector4F(0.0f);

	return subVector4F(mulValueVector4F(vector, d),
		mulValueVector4F(normal, eta *
		dotVector4F(normal, vector) + sqrtf(d)));
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
