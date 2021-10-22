// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
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

#include "mpgx/transformer.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <assert.h>

struct Transformer
{
	Transform* transforms;
	size_t transformCapacity;
	size_t transformCount;
};
struct Transform
{
	Transformer transformer;
	Vec3F position;
	Vec3F scale;
	Quat rotation;
	RotationType rotationType;
	Mat4F model;
	Transform parent;
	bool isActive;
};

Transformer createTransformer(size_t capacity)
{
	assert(capacity != 0);

	Transformer transformer = malloc(
		sizeof(struct Transformer));

	if (transformer == NULL)
		return NULL;

	Transform* transforms = malloc(
		sizeof(Transform) * capacity);

	if (transforms == NULL)
	{
		free(transformer);
		return NULL;
	}

	transformer->transforms = transforms;
	transformer->transformCapacity = capacity;
	transformer->transformCount = 0;
	return transformer;
}
void destroyTransformer(Transformer transformer)
{
	if (transformer == NULL)
		return;

	size_t transformCount = transformer->transformCount;
	Transform* transforms = transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
		free(transforms[i]);

	free(transforms);
	free(transformer);
}

bool isTransformerEmpty(Transformer transformer)
{
	assert(transformer != NULL);
	return transformer->transformCount == 0;
}

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	RotationType rotationType,
	Transform parent,
	bool isActive)
{
	assert(transformer != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);

	assert((parent == NULL) ||
		(parent != NULL &&
		transformer == parent->transformer));

	Transform transform = malloc(
		sizeof(struct Transform));

	if (transform == NULL)
		return NULL;

	transform->transformer = transformer;
	transform->position = position;
	transform->scale = scale;
	transform->rotation = rotation;
	transform->rotationType = rotationType;
	transform->model = identMat4F;
	transform->parent = parent;
	transform->isActive = isActive;

	size_t count = transformer->transformCount;

	if (count == transformer->transformCapacity)
	{
		size_t capacity = transformer->transformCapacity * 2;

		Transform* transforms = realloc(
			transformer->transforms,
			sizeof(Transform) * capacity);

		if (transforms == NULL)
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
	if (transform == NULL)
		return;

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

Transformer getTransformTransformer(
	Transform transform)
{
	assert(transform != NULL);
	return transform->transformer;
}

Vec3F getTransformPosition(
	Transform transform)
{
	assert(transform != NULL);
	return transform->position;
}
void setTransformPosition(
	Transform transform,
	Vec3F position)
{
	assert(transform != NULL);
	transform->position = position;
}

Vec3F getTransformScale(
	Transform transform)
{
	assert(transform != NULL);
	return transform->scale;
}
void setTransformScale(
	Transform transform,
	Vec3F scale)
{
	assert(transform != NULL);
	transform->scale = scale;
}

Quat getTransformRotation(
	Transform transform)
{
	assert(transform != NULL);
	return transform->rotation;
}
void setTransformRotation(
	Transform transform,
	Quat rotation)
{
	assert(transform != NULL);
	transform->rotation = rotation;
}

RotationType getTransformRotationType(
	Transform transform)
{
	assert(transform != NULL);
	return transform->rotationType;
}
void setTransformRotationType(
	Transform transform,
	RotationType rotationType)
{
	assert(transform != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);
	transform->rotationType = rotationType;
}

Transform getTransformParent(
	Transform transform)
{
	assert(transform != NULL);
	return transform->parent;
}
void setTransformParent(
	Transform transform,
	Transform parent)
{
	assert((parent == NULL) ||
		(parent != NULL &&
		transform->transformer ==
		parent->transformer));
	transform->parent = parent;
}

bool isTransformActive(
	Transform transform)
{
	assert(transform != NULL);
	return transform->isActive;
}
void setTransformActive(
	Transform transform,
	bool isActive)
{
	assert(transform != NULL);
	transform->isActive = isActive;
}

Mat4F getTransformModel(
	Transform transform)
{
	assert(transform != NULL);
	return transform->model;
}

void updateTransformer(
	Transformer transformer)
{
	assert(transformer != NULL);

	size_t transformCount = transformer->transformCount;

	if (transformCount == 0)
		return;

	Transform* transforms = transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		Transform transform = transforms[i];

		if (transform->isActive == false)
			continue;

		Transform parent = transform->parent;

		while (parent != NULL)
		{
			if (parent->isActive == false)
				goto CONTINUE_1;
			parent = parent->parent;
		}

		RotationType rotationType = transform->rotationType;

		Mat4F model = identMat4F;

		if (rotationType == SPIN_ROTATION_TYPE)
		{
			model = translateMat4F(
				model,
				transform->position);
			model = dotMat4F(
				model,
				getQuatMatF4(normQuat(transform->rotation)));
		}
		else if (rotationType == ORBIT_ROTATION_TYPE)
		{
			model = dotMat4F(
				model,
				getQuatMatF4(normQuat(transform->rotation)));
			model = translateMat4F(
				model,
				transform->position);
		}
		else
		{
			model = translateMat4F(
				model,
				transform->position);
		}

		model = scaleMat4F(
			model,
			transform->scale);
		transform->model = model;

	CONTINUE_1:
		continue;
	}

	for (size_t i = 0; i < transformCount; i++)
	{
		Transform transform = transforms[i];

		if (transform->isActive == false)
			continue;

		Transform parent = transform->parent;
		Mat4F model = transform->model;

		while (parent != NULL)
		{
			if (parent->isActive == false)
				goto CONTINUE_2;

			model = dotMat4F(
				parent->model,
				model);
			parent = parent->parent;
		}

		transform->model = model;

	CONTINUE_2:
		continue;
	}
}
