#pragma once
#include "cmmt/quaternion.h"
#include <stdint.h>

typedef struct Transformer* Transformer;
typedef struct Transform* Transform;

typedef enum RotationType
{
	NO_ROTATION_TYPE = 0,
	SPIN_ROTATION_TYPE = 1,
	ORBIT_ROTATION_TYPE = 2,
	ROTATION_TYPE_COUNT = 3,
} RotationType;

Transformer createTransformer();
void destroyTransformer(Transformer transformer);

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	uint8_t rotationType,
	Transform parent,
	bool isActive);
void destroyTransform(Transform transform);

Transformer getTransformTransformer(
	Transform transform);

Vec3F getTransformPosition(
	Transform transform);
void setTransformPosition(
	Transform transform,
	Vec3F position);

Vec3F getTransformScale(
	Transform transform);
void setTransformScale(
	Transform transform,
	Vec3F scale);

Quat getTransformRotation(
	Transform transform);
void setTransformRotation(
	Transform transform,
	Quat rotation);

uint8_t getTransformRotationType(
	Transform transform);
void setTransformRotationType(
	Transform transform,
	uint8_t rotationType);

Transform getTransformParent(
	Transform transform);
void setTransformParent(
	Transform transform,
	Transform parent);

bool isTransformActive(
	Transform transform);
void setTransformActive(
	Transform transform,
	bool isActive);

Mat4F getTransformModel(
	Transform transform);

void updateTransformer(
	Transformer transformer);
