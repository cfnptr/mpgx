#pragma once
#include "vector.h"

struct BoundingBox2F
{
	struct Vector2F minimum;
	struct Vector2F maximum;
};

inline static struct BoundingBox2F createBoundingBox2F(
	struct Vector2F minimum,
	struct Vector2F maximum)
{
	struct BoundingBox2F box;
	box.minimum = minimum;
	box.maximum = maximum;
	return box;
}

inline static bool isPointCollidingBoundingBox2F(
	struct BoundingBox2F box,
	struct Vector2F point)
{
	return
		box.minimum.x <= point.x &&
		box.maximum.x >= point.x &&
		box.minimum.y <= point.y &&
		box.maximum.y >= point.y;
}

inline static bool isCollidingBoundingBox2F(
	struct BoundingBox2F a,
	struct BoundingBox2F b)
{
	return
		a.minimum.x <= b.maximum.x &&
		a.maximum.x >= b.minimum.x &&
		a.minimum.y <= b.maximum.y &&
		a.maximum.y >= b.minimum.y;
}
