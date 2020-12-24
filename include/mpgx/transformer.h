#pragma once
#include "mpgx/quaternion.h"

struct Transformer;
struct Transform;

struct Transformer* createTransformer();

void destroyTransformer(
	struct Transformer* transformer);

struct Transform* createTransform(
	struct Transformer* transformer,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Matrix4F model,
	struct Transform* parent);
void destroyTransform(
	struct Transform* transform);

struct Vector3F getTransformPosition(
	const struct Transform* transform);
void setTransformPosition(
	struct Transform* transform,
	struct Vector3F position);

struct Vector3F getTransformScale(
	const struct Transform* transform);
void setTransformScale(
	struct Transform* transform,
	struct Vector3F scale);

struct Quaternion getTransformRotation(
	const struct Transform* transform);
void setTransformRotation(
	struct Transform* transform,
	struct Quaternion rotation);

struct Matrix4F getTransformModel(
	const struct Transform* transform);

void bakeTransformer(
	struct Transformer* transformer);
