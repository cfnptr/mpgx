// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// TODO: add thread pool to the constructor
// and update transformer acync using atomics

#include "mpgx/transformer.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <assert.h>

struct Transformer_T
{
	Transform* transforms;
	size_t transformCapacity;
	size_t transformCount;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};
struct Transform_T
{
	Transformer transformer;
	Transform parent;
	Mat4F model; // TODO: use Mat4x3F
	Quat rotation;
	Vec3F scale;
	Vec3F position;
	RotationType rotationType;
	bool isActive;
	bool isStatic;
};

inline static void updateTransformModel(
	Transform transform,
	bool checkParentActive)
{
	Vec3F position = transform->position;
	Quat rotation = transform->rotation;
	Transform parent = transform->parent;

	while (parent)
	{
		if (!parent->isActive & checkParentActive)
			return;

		rotation = normQuat(dotQuat(
			rotation, parent->rotation));
		position = addVec3F(parent->position,
			dotQuatVec3F(rotation, position));
		parent = parent->parent;
	}

	RotationType rotationType = transform->rotationType;

	Mat4F model;

	if (rotationType == SPIN_ROTATION_TYPE)
	{
		model = dotMat4F(translateMat4F(
			identMat4F, position),
			getQuatMatF4(normQuat(rotation)));
	}
	else if (rotationType == ORBIT_ROTATION_TYPE)
	{
		model = translateMat4F(dotMat4F(
			identMat4F,getQuatMatF4(
			normQuat(rotation))),position);
	}
	else
	{
		model = translateMat4F(identMat4F,position);
	}

	transform->model = scaleMat4F(model, transform->scale);
}

void destroyTransformer(Transformer transformer)
{
	if (!transformer)
		return;

	assert(transformer->transformCount == 0);
	assert(!transformer->isEnumerating);

	free(transformer->transforms);
	free(transformer);
}
Transformer createTransformer(size_t capacity)
{
	assert(capacity > 0);

	Transformer transformer = calloc(1,
		sizeof(Transformer_T));

	if (!transformer)
		return NULL;

	Transform* transforms = malloc(
		sizeof(Transform) * capacity);

	if (!transforms)
	{
		destroyTransformer(transformer);
		return NULL;
	}

	transformer->transforms = transforms;
	transformer->transformCapacity = capacity;
	transformer->transformCount = 0;
#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif
	return transformer;
}

void enumerateTransformer(
	Transformer transformer,
	void(*onItem)(Transform))
{
	assert(transformer);
	assert(onItem);

#ifndef NDEBUG
	transformer->isEnumerating = true;
#endif

	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (size_t i = 0; i < transformCount; i++)
		onItem(transforms[i]);

#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif
}
void destroyAllTransforms(
	Transformer transformer)
{
	assert(transformer);
	assert(!transformer->isEnumerating);

	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	if (transformCount == 0)
		return;

	for (size_t i = 0; i < transformCount; i++)
		free(transforms[i]);

	transformer->transformCount = 0;
}

void updateTransformer(Transformer transformer)
{
	assert(transformer);
	assert(!transformer->isEnumerating);

	size_t transformCount = transformer->transformCount;
	Transform* transforms = transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		Transform transform = transforms[i];

		if (!transform->isActive | transform->isStatic)
			continue;

		updateTransformModel(
			transform,
			true);
	}
}

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	RotationType rotationType,
	Transform parent,
	bool isActive,
	bool isStatic)
{
	assert(transformer);
	assert(rotationType < ROTATION_TYPE_COUNT);

	assert(!parent || (parent &&
		transformer == parent->transformer));
	assert(!transformer->isEnumerating);

	Transform transform = malloc(sizeof(Transform_T));

	if (!transform)
		return NULL;

	transform->transformer = transformer;
	transform->parent = parent;
	transform->rotation = rotation;
	transform->scale = scale;
	transform->position = position;
	transform->rotationType = rotationType;
	transform->isActive = isActive;
	transform->isStatic = isStatic;

	updateTransformModel(
		transform,
		false);

	size_t count = transformer->transformCount;

	if (count == transformer->transformCapacity)
	{
		size_t capacity = transformer->transformCapacity * 2;

		Transform* transforms = realloc(
			transformer->transforms,
			sizeof(Transform) * capacity);

		if (!transforms)
		{
			free(transform);
			return NULL;
		}

		transformer->transforms = transforms;
		transformer->transformCapacity = capacity;
	}

	transformer->transforms[count] = transform;
	transformer->transformCount = count + 1;
	return transform;
}
void destroyTransform(Transform transform)
{
	if (!transform)
		return;

	assert(!transform->transformer->isEnumerating);

	Transformer transformer = transform->transformer;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (size_t i = 0; i < transformCount; i++)
	{
		if (transforms[i] != transform)
			continue;

		for (size_t j = i + 1; j < transformCount; j++)
			transforms[j - 1] = transforms[j];

		free(transform);
		transformer->transformCount--;
		return;
	}

	abort();
}

void updateTransform(
	Transform transform,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	RotationType rotationType,
	Transform parent,
	bool isActive,
	bool isStatic)
{
	assert(transform);
	assert(rotationType < ROTATION_TYPE_COUNT);

	assert(!parent || (parent &&
		transform->transformer == parent->transformer));

	transform->parent = parent;
	transform->rotation = rotation;
	transform->scale = scale;
	transform->position = position;
	transform->rotationType = rotationType;
	transform->isActive = isActive;
	transform->isStatic = isStatic;

	updateTransformModel(
		transform,
		false);
}

Transformer getTransformTransformer(Transform transform)
{
	assert(transform);
	return transform->transformer;
}

Vec3F getTransformPosition(
	Transform transform)
{
	assert(transform);
	return transform->position;
}
void setTransformPosition(
	Transform transform,
	Vec3F position)
{
	assert(transform);
	assert(!transform->isStatic);
	transform->position = position;
}

Vec3F getTransformScale(
	Transform transform)
{
	assert(transform);
	return transform->scale;
}
void setTransformScale(
	Transform transform,
	Vec3F scale)
{
	assert(transform);
	assert(!transform->isStatic);
	transform->scale = scale;
}

Quat getTransformRotation(
	Transform transform)
{
	assert(transform);
	return transform->rotation;
}
void setTransformRotation(
	Transform transform,
	Quat rotation)
{
	assert(transform);
	assert(!transform->isStatic);
	transform->rotation = rotation;
}

RotationType getTransformRotationType(
	Transform transform)
{
	assert(transform);
	return transform->rotationType;
}
void setTransformRotationType(
	Transform transform,
	RotationType rotationType)
{
	assert(transform);
	assert(rotationType < ROTATION_TYPE_COUNT);
	assert(!transform->isStatic);
	transform->rotationType = rotationType;
}

Transform getTransformParent(
	Transform transform)
{
	assert(transform);
	return transform->parent;
}
void setTransformParent(
	Transform transform,
	Transform parent)
{
	assert(!parent || (parent &&
		transform->transformer ==
		parent->transformer));
	assert(!transform->isStatic);
	transform->parent = parent;
}

bool isTransformActive(
	Transform transform)
{
	assert(transform);
	return transform->isActive;
}
void setTransformActive(
	Transform transform,
	bool isActive)
{
	assert(transform);
	assert(!transform->isStatic);
	transform->isActive = isActive;
}

bool isTransformStatic(Transform transform)
{
	assert(transform);
	return transform->isStatic;
}
Mat4F getTransformModel(Transform transform)
{
	assert(transform);
	return transform->model;
}
