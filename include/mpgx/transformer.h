#pragma once
#include "cmmt/quaternion.h"
#include <stdint.h>

struct Transformer;
struct Transform;

enum ROTATION_TYPE
{
	SPIN_ROTATION_TYPE = 0,
	ORBIT_ROTATION_TYPE = 1,
	ROTATION_TYPE_COUNT = ORBIT_ROTATION_TYPE + 1,
};

// TODO: move renderer draw to the transfromer update
// also interface

struct Transformer* createTransformer();

void destroyTransformer(
	struct Transformer* transformer);

struct Transform* createTransform(
	struct Transformer* transformer,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Transform* parent);
void destroyTransform(
	struct Transform* transform);

struct Transformer* getTransformTransformer(
	const struct Transform* transform);

struct Vec3F getTransformPosition(
	const struct Transform* transform);
void setTransformPosition(
	struct Transform* transform,
	struct Vec3F position);

struct Vec3F getTransformScale(
	const struct Transform* transform);
void setTransformScale(
	struct Transform* transform,
	struct Vec3F scale);

struct Quat getTransformRotation(
	const struct Transform* transform);
void setTransformRotation(
	struct Transform* transform,
	struct Quat rotation);

uint8_t getTransformRotationType(
	const struct Transform* transform);
void setTransformRotationType(
	struct Transform* transform,
	uint8_t rotationType);

struct Transform* getTransformParent(
	const struct Transform* transform);
void setTransformParent(
	struct Transform* transform,
	struct Transform* parent);

struct Mat4F getTransformModel(
	const struct Transform* transform);

void executeTransformer(
	struct Transformer* transformer);
