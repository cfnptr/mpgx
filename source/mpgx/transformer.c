#include "mpgx/transformer.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <assert.h>

struct Transformer
{
	Transform** transforms;
	size_t transformCapacity;
	size_t transformCount;
};
struct Transform
{
	Transformer* transformer;
	Vector3F position;
	Vector3F scale;
	Quaternion rotation;
	uint8_t rotationType;
	Matrix4F model;
	Transform* parent;
};

Transformer* createTransformer()
{
	Transformer* transformer = malloc(
		sizeof(Transformer));

	if (transformer == NULL)
		return NULL;

	Transform** transforms = malloc(
		sizeof(Transform*));

	if (transforms == NULL)
	{
		free(transformer);
		return NULL;
	}

	transformer->transforms = transforms;
	transformer->transformCapacity = 1;
	transformer->transformCount = 0;
	return transformer;
}

void destroyTransformer(
	Transformer* transformer)
{
	if (transformer == NULL)
		return;

	size_t transformCount =
		transformer->transformCount;
	Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
		free(transforms[i]);

	free(transforms);
	free(transformer);
}

Transform* createTransform(
	Transformer* transformer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Transform* parent)
{
	assert(transformer != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);

#ifndef NDEBUG
	if (parent != NULL)
		assert(transformer == parent->transformer);
#endif

	Transform* transform = malloc(
		sizeof(Transform));

	if (transform == NULL)
		return NULL;

	transform->transformer = transformer;
	transform->position = position;
	transform->scale = scale;
	transform->rotation = rotation;
	transform->rotationType = rotationType;
	transform->model = identMat4F();
	transform->parent = parent;

	if (transformer->transformCount ==
		transformer->transformCapacity)
	{
		size_t capacity =
			transformer->transformCapacity * 2;
		Transform** transforms = realloc(
			transformer->transforms,
			capacity * sizeof(Transform*));

		if (transforms == NULL)
			return NULL;

		transformer->transforms = transforms;
		transformer->transformCapacity = capacity;
	}

	transformer->transforms[
		transformer->transformCount] = transform;
	transformer->transformCount++;
	return transform;
}
void destroyTransform(
	Transform* transform)
{
	if (transform == NULL)
		return;

	Transformer* transformer =
		transform->transformer;
	size_t transformCount =
		transformer->transformCount;
	Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		if (transforms[i] == transform)
		{
			for (size_t j = i + 1; j < transformCount; j++)
				transforms[j - 1] = transforms[j];

			free(transform);
			transformer->transformCount--;
			return;
		}
	}

	abort();
}

Transformer* getTransformTransformer(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->transformer;
}

Vector3F getTransformPosition(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->position;
}
void setTransformPosition(
	Transform* transform,
	Vector3F position)
{
	assert(transform != NULL);
	transform->position = position;
}

Vector3F getTransformScale(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->scale;
}
void setTransformScale(
	Transform* transform,
	Vector3F scale)
{
	assert(transform != NULL);
	transform->scale = scale;
}

Quaternion getTransformRotation(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->rotation;
}
void setTransformRotation(
	Transform* transform,
	Quaternion rotation)
{
	assert(transform != NULL);
	transform->rotation = rotation;
}

uint8_t getTransformRotationType(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->rotationType;
}
void setTransformRotationType(
	Transform* transform,
	uint8_t rotationType)
{
	assert(transform != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);
	transform->rotationType = rotationType;
}

Transform* getTransformParent(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->parent;
}
void setTransformParent(
	Transform* transform,
	Transform* parent)
{
#ifndef NDEBUG
	if (parent != NULL)
	{
		assert(transform->transformer ==
			parent->transformer);
	}
#endif

	transform->parent = parent;
}

Matrix4F getTransformModel(
	const Transform* transform)
{
	assert(transform != NULL);
	return transform->model;
}

void executeTransformer(
	Transformer* transformer)
{
	assert(transformer != NULL);

	size_t transformCount =
		transformer->transformCount;
	Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		Transform* transform = transforms[i];
		uint8_t rotationType = transform->rotationType;

		Matrix4F model = identMat4F();

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
			abort();
		}

		model = scaleMat4F(
			model,
			transform->scale);
		transform->model = model;
	}

	for (size_t i = 0; i < transformCount; i++)
	{
		Transform* transform = transforms[i];
		Transform* parent = transform->parent;

		if (parent == NULL)
			continue;

		Matrix4F model = transform->model;

		while (parent != NULL)
		{
			model = dotMat4F(
				parent->model, // TODO: check if correct
				model);
			parent = parent->parent;
		}

		transform->model = model;
	}
}