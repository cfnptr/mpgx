#pragma once
#include "cmmt/quaternion.h"
#include <stdint.h>

typedef struct Transformer Transformer;
typedef struct Transform Transform;

typedef enum ROTATION_TYPE
{
	SPIN_ROTATION_TYPE,
	ORBIT_ROTATION_TYPE,
	ROTATION_TYPE_COUNT = ORBIT_ROTATION_TYPE + 1,
} ROTATION_TYPE;

// TODO: move renderer draw to the transfromer update
// also interface

Transformer* createTransformer();

void destroyTransformer(
	Transformer* transformer);

Transform* createTransform(
	Transformer* transformer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Transform* parent);
void destroyTransform(
	Transform* transform);

Transformer* getTransformTransformer(
	const Transform* transform);

Vector3F getTransformPosition(
	const Transform* transform);
void setTransformPosition(
	Transform* transform,
	Vector3F position);

Vector3F getTransformScale(
	const Transform* transform);
void setTransformScale(
	Transform* transform,
	Vector3F scale);

Quaternion getTransformRotation(
	const Transform* transform);
void setTransformRotation(
	Transform* transform,
	Quaternion rotation);

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

Matrix4F getTransformModel(
	const Transform* transform);

void executeTransformer(
	Transformer* transformer);
