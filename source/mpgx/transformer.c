#include "mpgx/transformer.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <assert.h>

struct Transformer
{
	struct Transform** transforms;
	size_t transformCapacity;
	size_t transformCount;
};
struct Transform
{
	struct Transformer* transformer;
	struct Vec3F position;
	struct Vec3F scale;
	struct Quat rotation;
	uint8_t rotationType;
	struct Mat4F model;
	struct Transform* parent;
};

struct Transformer* createTransformer()
{
	struct Transformer* transformer = malloc(
		sizeof(struct Transformer));

	if (transformer == NULL)
		return NULL;

	struct Transform** transforms = malloc(
		sizeof(struct Transform*));

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
	struct Transformer* transformer)
{
	if (transformer == NULL)
		return;

	size_t transformCount =
		transformer->transformCount;
	struct Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
		free(transforms[i]);

	free(transforms);
	free(transformer);
}

struct Transform* createTransform(
	struct Transformer* transformer,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Transform* parent)
{
	assert(transformer != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);

#ifndef NDEBUG
	if (parent != NULL)
		assert(transformer == parent->transformer);
#endif

	struct Transform* transform = malloc(
		sizeof(struct Transform));

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
		struct Transform** transforms = realloc(
			transformer->transforms,
			capacity * sizeof(struct Transform*));

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
	struct Transform* transform)
{
	if (transform == NULL)
		return;

	struct Transformer* transformer =
		transform->transformer;
	size_t transformCount =
		transformer->transformCount;
	struct Transform** transforms =
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

struct Transformer* getTransformTransformer(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->transformer;
}

struct Vec3F getTransformPosition(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->position;
}
void setTransformPosition(
	struct Transform* transform,
	struct Vec3F position)
{
	assert(transform != NULL);
	transform->position = position;
}

struct Vec3F getTransformScale(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->scale;
}
void setTransformScale(
	struct Transform* transform,
	struct Vec3F scale)
{
	assert(transform != NULL);
	transform->scale = scale;
}

struct Quat getTransformRotation(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->rotation;
}
void setTransformRotation(
	struct Transform* transform,
	struct Quat rotation)
{
	assert(transform != NULL);
	transform->rotation = rotation;
}

uint8_t getTransformRotationType(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->rotationType;
}
void setTransformRotationType(
	struct Transform* transform,
	uint8_t rotationType)
{
	assert(transform != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);
	transform->rotationType = rotationType;
}

struct Transform* getTransformParent(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->parent;
}
void setTransformParent(
	struct Transform* transform,
	struct Transform* parent)
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

struct Mat4F getTransformModel(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->model;
}

void executeTransformer(
	struct Transformer* transformer)
{
	assert(transformer != NULL);

	size_t transformCount =
		transformer->transformCount;
	struct Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		struct Transform* transform = transforms[i];
		uint8_t rotationType = transform->rotationType;

		struct Mat4F model = identMat4F();

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
		struct Transform* transform =
			transforms[i];
		struct Transform* parent =
			transform->parent;

		if (parent == NULL)
			continue;

		struct Mat4F model =
			transform->model;

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