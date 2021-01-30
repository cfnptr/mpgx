#include "mpgx/transformer.h"

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
	struct Vector3F position;
	struct Vector3F scale;
	struct Quaternion rotation;
	struct Matrix4F model;
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
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent)
{
	assert(transformer != NULL);

	struct Transform* transform = malloc(
		sizeof(struct Transform));

	if (transform == NULL)
		return NULL;

	transform->transformer = transformer;
	transform->position = position;
	transform->scale = scale;
	transform->rotation = rotation;
	transform->model = createIdentityMatrix4F();
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
		{
			free(transform);
			return NULL;
		}

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

struct Vector3F getTransformPosition(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->position;
}
void setTransformPosition(
	struct Transform* transform,
	struct Vector3F position)
{
	assert(transform != NULL);
	transform->position = position;
}

struct Vector3F getTransformScale(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->scale;
}
void setTransformScale(
	struct Transform* transform,
	struct Vector3F scale)
{
	assert(transform != NULL);
	transform->scale = scale;
}

struct Quaternion getTransformRotation(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->rotation;
}
void setTransformRotation(
	struct Transform* transform,
	struct Quaternion rotation)
{
	assert(transform != NULL);
	transform->rotation = rotation;
}

struct Matrix4F getTransformModel(
	const struct Transform* transform)
{
	assert(transform != NULL);
	return transform->model;
}

void executeTransformer(
	struct Transformer* transformer)
{
	size_t transformCount =
		transformer->transformCount;
	struct Transform** transforms =
		transformer->transforms;

	for (size_t i = 0; i < transformCount; i++)
	{
		struct Transform* transform =
			transforms[i];

		struct Matrix4F model =
			createIdentityMatrix4F();
		model = translateMatrix4F(
			model,
			transform->position);
		model = dotMatrix4F(
			getQuaternionMatrixF4(transform->rotation),
			model);
		model = scaleMatrix4F(
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

		struct Matrix4F model =
			transform->model;

		while (parent != NULL)
		{
			model = dotMatrix4F(
				model,
				parent->model);
			parent = parent->parent;
		}

		transform->model = model;
	}
}