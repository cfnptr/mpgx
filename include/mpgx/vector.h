#pragma once
#include <math.h>
#include <stdbool.h>

struct Vector2F
{
	float x, y;
};
struct Vector3F
{
	float x, y, z;
};
struct Vector4F
{
	float x, y, z, w;
};

struct Vector2I
{
	int x, y;
};
struct Vector3I
{
	int x, y, z;
};
struct Vector4I
{
	int x, y, z, w;
};

inline static struct Vector2F createVector2F(
	float x,
	float y)
{
	struct Vector2F vector;
	vector.x = x;
	vector.y = y;
	return vector;
}
inline static struct Vector2F createValueVector2F(
	float value)
{
	struct Vector2F vector;
	vector.x = value;
	vector.y = value;
	return vector;
}
inline static struct Vector2F createZeroVector2F()
{
	struct Vector2F vector;
	vector.x = 0.0f;
	vector.y = 0.0f;
	return vector;
}
inline static struct Vector2F createOneVector2F()
{
	struct Vector2F vector;
	vector.x = 1.0f;
	vector.y = 1.0f;
	return vector;
}
inline static struct Vector2F createMinusOneVector2F()
{
	struct Vector2F vector;
	vector.x = -1.0f;
	vector.y = -1.0f;
	return vector;
}

inline static struct Vector2F addVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}
inline static struct Vector2F subVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}
inline static struct Vector2F mulVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}
inline static struct Vector2F divVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

inline static struct Vector2F addValueVector2F(
	struct Vector2F vector,
	float value)
{
	vector.x += value;
	vector.y += value;
	return vector;
}
inline static struct Vector2F subValueVector2F(
	struct Vector2F vector,
	float value)
{
	vector.x -= value;
	vector.y -= value;
	return vector;
}
inline static struct Vector2F mulValueVector2F(
	struct Vector2F vector,
	float value)
{
	vector.x *= value;
	vector.y *= value;
	return vector;
}
inline static struct Vector2F divValueVector2F(
	struct Vector2F vector,
	float value)
{
	vector.x /= value;
	vector.y /= value;
	return vector;
}

inline static struct Vector2F negVector2F(
	struct Vector2F vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	return vector;
}

inline static float dotVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	return
		(a.x * b.x) +
		(a.y * b.y);
}
inline static float lengthVector2F(
	struct Vector2F vector)
{
	return sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y));
}
inline static float distanceVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	return sqrtf(
		((a.x - b.x) * (a.x - b.x)) +
		((a.y - b.y) * (a.y - b.y)));
}
inline static struct Vector2F normalizeVector2F(
	struct Vector2F vector)
{
	float length = sqrtf(
		(vector.x * vector.x) +
		(vector.y * vector.y));

	vector.x /= length;
	vector.y /= length;
	return vector;
}
inline static struct Vector2F reflectVector2F(
	struct Vector2F vector,
	struct Vector2F normal)
{
	float dot =
		(normal.x * vector.x) +
		(normal.y * vector.y) * 2.0f;

	normal.x *= dot;
	normal.y *= dot;

	vector.x -= normal.x;
	vector.y -= normal.y;
	return vector;
}

inline static bool compareVector2F(
	struct Vector2F a,
	struct Vector2F b)
{
	return
		a.x == b.x &&
		a.y == b.y;
}

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
inline static struct Vector3F createZeroVector3F()
{
	struct Vector3F vector;
	vector.x = 0.0f;
	vector.y = 0.0f;
	vector.z = 0.0f;
	return vector;
}
inline static struct Vector3F createOneVector3F()
{
	struct Vector3F vector;
	vector.x = 1.0f;
	vector.y = 1.0f;
	vector.z = 1.0f;
	return vector;
}
inline static struct Vector3F createMinusOneVector3F()
{
	struct Vector3F vector;
	vector.x = -1.0f;
	vector.y = -1.0f;
	vector.z = -1.0f;
	return vector;
}

inline static struct Vector3F createLeftVector3F()
{
	struct Vector3F vector;
	vector.x = -1.0f;
	vector.y = 0.0f;
	vector.z = 0.0f;
	return vector;
}
inline static struct Vector3F createRightVector3F()
{
	struct Vector3F vector;
	vector.x = 1.0f;
	vector.y = 0.0f;
	vector.z = 0.0f;
	return vector;
}
inline static struct Vector3F createBottomVector3F()
{
	struct Vector3F vector;
	vector.x = 0.0f;
	vector.y = -1.0f;
	vector.z = 0.0f;
	return vector;
}
inline static struct Vector3F createTopVector3F()
{
	struct Vector3F vector;
	vector.x = 0.0f;
	vector.y = 1.0f;
	vector.z = 0.0f;
	return vector;
}
inline static struct Vector3F createBackVector3F()
{
	struct Vector3F vector;
	vector.x = 0.0f;
	vector.y = 0.0f;
	vector.z = -1.0f;
	return vector;
}
inline static struct Vector3F createFrontVector3F()
{
	struct Vector3F vector;
	vector.x = 0.0f;
	vector.y = 0.0f;
	vector.z = 1.0f;
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
		(a.x * b.x) +
		(a.y * b.y) +
		(a.z * b.z);
}
inline static struct Vector3F crossVector3F(
	struct Vector3F a,
	struct Vector3F b)
{
	a.x = a.y * b.z - a.z * b.y;
	a.y = a.z * b.x - a.x * b.z;
	a.z = a.x * b.y - a.y * b.x;
	return a;
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
		(normal.x * vector.x) +
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
inline static struct Vector4F createZeroVector4F()
{
	struct Vector4F vector;
	vector.x = 0.0f;
	vector.y = 0.0f;
	vector.z = 0.0f;
	vector.w = 0.0f;
	return vector;
}
inline static struct Vector4F createOneVector4F()
{
	struct Vector4F vector;
	vector.x = 1.0f;
	vector.y = 1.0f;
	vector.z = 1.0f;
	vector.w = 1.0f;
	return vector;
}
inline static struct Vector4F createMinusOneVector4F()
{
	struct Vector4F vector;
	vector.x = -1.0f;
	vector.y = -1.0f;
	vector.z = -1.0f;
	vector.w = -1.0f;
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
		(a.x * b.x) +
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
		(normal.x * vector.x) +
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

inline static struct Vector2I createVector2I(
	int x,
	int y)
{
	struct Vector2I vector;
	vector.x = x;
	vector.y = y;
	return vector;
}
inline static struct Vector2I createValueVector2I(
	int value)
{
	struct Vector2I vector;
	vector.x = value;
	vector.y = value;
	return vector;
}
inline static struct Vector2I createZeroVector2I()
{
	struct Vector2I vector;
	vector.x = 0;
	vector.y = 0;
	return vector;
}
inline static struct Vector2I createOneVector2I()
{
	struct Vector2I vector;
	vector.x = 1;
	vector.y = 1;
	return vector;
}
inline static struct Vector2I createMinusOneVector2I()
{
	struct Vector2I vector;
	vector.x = -1;
	vector.y = -1;
	return vector;
}

inline static struct Vector2I addVector2I(
	struct Vector2I a,
	struct Vector2I b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}
inline static struct Vector2I subVector2I(
	struct Vector2I a,
	struct Vector2I b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}
inline static struct Vector2I mulVector2I(
	struct Vector2I a,
	struct Vector2I b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}
inline static struct Vector2I divVector2I(
	struct Vector2I a,
	struct Vector2I b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

inline static struct Vector2I addValueVector2I(
	struct Vector2I vector,
	int value)
{
	vector.x += value;
	vector.y += value;
	return vector;
}
inline static struct Vector2I subValueVector2I(
	struct Vector2I vector,
	int value)
{
	vector.x -= value;
	vector.y -= value;
	return vector;
}
inline static struct Vector2I mulValueVector2I(
	struct Vector2I vector,
	int value)
{
	vector.x *= value;
	vector.y *= value;
	return vector;
}
inline static struct Vector2I divValueVector2I(
	struct Vector2I vector,
	int value)
{
	vector.x /= value;
	vector.y /= value;
	return vector;
}

inline static struct Vector2I negVector2I(
	struct Vector2I vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	return vector;
}

inline static bool compareVector2I(
	struct Vector2I a,
	struct Vector2I b)
{
	return
		a.x == b.x &&
		a.y == b.y;
}

inline static struct Vector3I createVector3I(
	int x,
	int y,
	int z)
{
	struct Vector3I vector;
	vector.x = x;
	vector.y = y;
	vector.z = z;
	return vector;
}
inline static struct Vector3I createValueVector3I(
	int value)
{
	struct Vector3I vector;
	vector.x = value;
	vector.y = value;
	vector.z = value;
	return vector;
}
inline static struct Vector3I createZeroVector3I()
{
	struct Vector3I vector;
	vector.x = 0;
	vector.y = 0;
	vector.z = 0;
	return vector;
}
inline static struct Vector3I createOneVector3I()
{
	struct Vector3I vector;
	vector.x = 1;
	vector.y = 1;
	vector.z = 1;
	return vector;
}
inline static struct Vector3I createMinusOneVector3I()
{
	struct Vector3I vector;
	vector.x = -1;
	vector.y = -1;
	vector.z = -1;
	return vector;
}

inline static struct Vector3I createLeftVector3I()
{
	struct Vector3I vector;
	vector.x = -1;
	vector.y = 0;
	vector.z = 0;
	return vector;
}
inline static struct Vector3I createRightVector3I()
{
	struct Vector3I vector;
	vector.x = 1;
	vector.y = 0;
	vector.z = 0;
	return vector;
}
inline static struct Vector3I createBottomVector3I()
{
	struct Vector3I vector;
	vector.x = 0;
	vector.y = -1;
	vector.z = 0;
	return vector;
}
inline static struct Vector3I createTopVector3I()
{
	struct Vector3I vector;
	vector.x = 0;
	vector.y = 1;
	vector.z = 0;
	return vector;
}
inline static struct Vector3F createBackVector3I()
{
	struct Vector3F vector;
	vector.x = 0;
	vector.y = 0;
	vector.z = -1;
	return vector;
}
inline static struct Vector3F createFrontVector3I()
{
	struct Vector3F vector;
	vector.x = 0;
	vector.y = 0;
	vector.z = 1;
	return vector;
}

inline static struct Vector3I addVector3I(
	struct Vector3I a,
	struct Vector3I b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
inline static struct Vector3I subVector3I(
	struct Vector3I a,
	struct Vector3I b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}
inline static struct Vector3I mulVector3I(
	struct Vector3I a,
	struct Vector3I b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}
inline static struct Vector3I divVector3I(
	struct Vector3I a,
	struct Vector3I b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

inline static struct Vector3I addValueVector3I(
	struct Vector3I vector,
	int value)
{
	vector.x += value;
	vector.y += value;
	vector.z += value;
	return vector;
}
inline static struct Vector3I subValueVector3I(
	struct Vector3I vector,
	int value)
{
	vector.x -= value;
	vector.y -= value;
	vector.z -= value;
	return vector;
}
inline static struct Vector3I mulValueVector3I(
	struct Vector3I vector,
	int value)
{
	vector.x *= value;
	vector.y *= value;
	vector.z *= value;
	return vector;
}
inline static struct Vector3I divValueVector3I(
	struct Vector3I vector,
	int value)
{
	vector.x /= value;
	vector.y /= value;
	vector.z /= value;
	return vector;
}

inline static struct Vector3I negVector3I(
	struct Vector3I vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	vector.z = -vector.z;
	return vector;
}

inline static bool compareVector3I(
	struct Vector3I a,
	struct Vector3I b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z;
}

inline static struct Vector4I createVector4I(
	int x,
	int y,
	int z,
	int w)
{
	struct Vector4I vector;
	vector.x = x;
	vector.y = y;
	vector.z = z;
	vector.w = w;
	return vector;
}
inline static struct Vector4I createValueVector4I(
	int value)
{
	struct Vector4I vector;
	vector.x = value;
	vector.y = value;
	vector.z = value;
	vector.w = value;
	return vector;
}
inline static struct Vector4I createZeroVector4I()
{
	struct Vector4I vector;
	vector.x = 0;
	vector.y = 0;
	vector.z = 0;
	vector.w = 0;
	return vector;
}
inline static struct Vector4I createOneVector4I()
{
	struct Vector4I vector;
	vector.x = 1;
	vector.y = 1;
	vector.z = 1;
	vector.w = 1;
	return vector;
}
inline static struct Vector4I createMinusOneVector4I()
{
	struct Vector4I vector;
	vector.x = -1;
	vector.y = -1;
	vector.z = -1;
	vector.w = -1;
	return vector;
}

inline static struct Vector4I addVector4I(
	struct Vector4I a,
	struct Vector4I b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}
inline static struct Vector4I subVector4I(
	struct Vector4I a,
	struct Vector4I b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}
inline static struct Vector4I mulVector4I(
	struct Vector4I a,
	struct Vector4I b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
	return a;
}
inline static struct Vector4I divVector4I(
	struct Vector4I a,
	struct Vector4I b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
	return a;
}

inline static struct Vector4I addValueVector4I(
	struct Vector4I vector,
	int value)
{
	vector.x += value;
	vector.y += value;
	vector.z += value;
	vector.w += value;
	return vector;
}
inline static struct Vector4I subValueVector4I(
	struct Vector4I vector,
	int value)
{
	vector.x -= value;
	vector.y -= value;
	vector.z -= value;
	vector.w -= value;
	return vector;
}
inline static struct Vector4I mulValueVector4I(
	struct Vector4I vector,
	int value)
{
	vector.x *= value;
	vector.y *= value;
	vector.z *= value;
	vector.w *= value;
	return vector;
}
inline static struct Vector4I divValueVector4I(
	struct Vector4I vector,
	int value)
{
	vector.x /= value;
	vector.y /= value;
	vector.z /= value;
	vector.w /= value;
	return vector;
}

inline static struct Vector4I negVector4I(
	struct Vector4I vector)
{
	vector.x = -vector.x;
	vector.y = -vector.y;
	vector.z = -vector.z;
	vector.w = -vector.w;
	return vector;
}

inline static bool compareVector4I(
	struct Vector4I a,
	struct Vector4I b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z &&
		a.w == b.w;
}
