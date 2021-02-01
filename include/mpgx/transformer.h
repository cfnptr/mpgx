#pragma once
#include "mpgx/quaternion.h"

struct Transformer;
struct Transform;

// TODO: move renderer draw to the transfromer update
// also interface

struct Transformer* createTransformer();

void destroyTransformer(
	struct Transformer* transformer);

struct Transform* createTransform(
	struct Transformer* transformer,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent);
void destroyTransform(
	struct Transform* transform);

struct Transformer* getTransformTransformer(
	const struct Transform* transform);

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

struct Transform* getTransformParent(
	const struct Transform* transform);
void setTransformParent(
	struct Transform* transform,
	struct Transform* parent);

struct Matrix4F getTransformModel(
	const struct Transform* transform);

void executeTransformer(
	struct Transformer* transformer);
