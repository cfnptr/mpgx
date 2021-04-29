#pragma once
#include "cmmt/quaternion.h"
#include <stdint.h>

typedef struct Transformer Transformer;
typedef struct Transform Transform;

typedef enum ROTATION_TYPE
{
	NO_ROTATION_TYPE,
	SPIN_ROTATION_TYPE,
	ORBIT_ROTATION_TYPE,
	ROTATION_TYPE_COUNT,
} ROTATION_TYPE;

Transformer* createTransformer();
void destroyTransformer(Transformer* transformer);

Transform* createTransform(
	Transformer* transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	uint8_t rotationType,
	Transform* parent,
	bool isActive);
void destroyTransform(Transform* transform);

Transformer* getTransformTransformer(
	const Transform* transform);

Vec3F getTransformPosition(
	const Transform* transform);
void setTransformPosition(
	Transform* transform,
	Vec3F position);

Vec3F getTransformScale(
	const Transform* transform);
void setTransformScale(
	Transform* transform,
	Vec3F scale);

Quat getTransformRotation(
	const Transform* transform);
void setTransformRotation(
	Transform* transform,
	Quat rotation);

uint8_t getTransformRotationType(
	const Transform* transform);
void setTransformRotationType(
	Transform* transform,
	uint8_t rotationType);

Transform* getTransformParent(
	const Transform* transform);
void setTransformParent(
	Transform* transform,
	Transform* parent);

bool isTransformActive(
	const Transform* transform);
void setTransformActive(
	Transform* transform,
	bool isActive);

Mat4F getTransformModel(
	const Transform* transform);

void updateTransformer(
	Transformer* transformer);
