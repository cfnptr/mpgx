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
	struct Matrix4F model,
	struct Transform* parent)
{
	assert(transformer != NULL);

	struct Transform* transform =
		malloc(sizeof(struct Transform));

	if (transform == NULL)
		return NULL;

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

	transform->transformer = transformer;
	transform->position = position;
	transform->scale = scale;
	transform->rotation = rotation;
	transform->model = model;
	transform->parent = parent;

	transformer->transforms[
		transformer->transformCount] = transform;
	transformer->transformCount++;
	return transform;
}
void destroyTransform(
	struct Transform* transform)
{
	// TODO: use binary search instead

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
		if (transform == transforms[i])
		{
			for (size_t j = i + 1; j < transformCount; j++)
				transforms[j - 1] = transforms[j];

			transformer->transformCount--;
			free(transform);
			return;
		}
	}

	abort();
}

void bakeTransformer(
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
		model = mulMatrix4F(model,
			translateMatrix4F(model, transform->position));
		model = mulMatrix4F(model,
			getQuaternionMatrixF4(transform->rotation));
		model = mulMatrix4F(model,
			scaleMatrix4F(model, transform->scale));
		transform->model = model;
	}

	// TODO:
	// multiply by parent matrices
	// for ...
}