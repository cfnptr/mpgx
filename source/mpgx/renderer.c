#include "mpgx/renderer.h"

#include <assert.h>
#include <string.h>

struct Renderer
{
	Pipeline* pipeline;
	Transformer* transformer;
	bool ascendingSort;
	Transform* parent;
	Render** renders;
	size_t renderCapacity;
	size_t renderCount;
};
struct Render
{
	Renderer* renderer;
	bool draw;
	Transform* transform;
	Render* parent;
	DestroyRender destroyFunction;
	RenderCommand renderFunction;
	void* handle;
};

static int ascendCompareRender(
	const void* a,
	const void* b)
{
	Render* render =
		*(Render**)a;
	Vector3F renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceA = distPowVec3F(
		getTransformPosition(render->renderer->parent),
		renderPosition);

	render =
		*(Render**)b;
	renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceB = distPowVec3F(
		getTransformPosition(render->renderer->parent),
		renderPosition);

	if (distanceA < distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA > distanceB)
		return 1;

	abort();
}
static int descendCompareRender(
	const void* a,
	const void* b)
{
	Render* render =
		*(Render**)a;
	Vector3F renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceA = distPowVec3F(
		getTransformPosition(render->renderer->parent),
		renderPosition);

	render =
		*(Render**)b;
	renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceB = distPowVec3F(
		getTransformPosition(render->renderer->parent),
		renderPosition);

	if (distanceA > distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA < distanceB)
		return 1;

	abort();
}

Renderer* createRenderer(
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSort,
	Transform* parent)
{
	assert(pipeline != NULL);
	assert(transformer != NULL);

#ifndef NDEBUG
	if (parent != NULL)
		assert(getTransformTransformer(parent) == transformer);
#endif

	Renderer* renderer = malloc(
		sizeof(Renderer));

	if (renderer == NULL)
		return NULL;

	Render** renders = malloc(
		sizeof(Render*));

	if (renders == NULL)
	{
		free(renderer);
		return NULL;
	}

	renderer->pipeline = pipeline;
	renderer->transformer = transformer;
	renderer->ascendingSort = ascendingSort;
	renderer->parent = parent;
	renderer->renders = renders;
	renderer->renderCapacity = 1;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(
	Renderer* renderer)
{
	if (renderer == NULL)
		return;

	size_t renderCount =
		renderer->renderCount;
	Render** renders =
		renderer->renders;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];
		render->destroyFunction(render->handle);
		free(render);
	}

	free(renders);
	free(renderer);
}

Pipeline* getRendererPipeline(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->pipeline;
}
Transformer* getRendererTransformer(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->transformer;
}
Transform* getRendererTransform(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->parent;
}

bool getRendererSorting(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->ascendingSort;
}
void setRendererSorting(
	Renderer* renderer,
	bool ascendingSorting)
{
	assert(renderer != NULL);
	renderer->ascendingSort = ascendingSorting;
}

void executeRenderer(
	Renderer* renderer,
	Camera camera)
{
	assert(renderer != NULL);

	size_t renderCount =
		renderer->renderCount;
	Render** renders =
		renderer->renders;

	if (renderCount == 0)
		return;

	if (renderer->ascendingSort == true)
	{
		qsort(
			renders,
			renderCount,
			sizeof(Render*),
			ascendCompareRender);
	}
	else
	{
		qsort(
			renders,
			renderCount,
			sizeof(Render*),
			descendCompareRender);
	}

	Pipeline* pipeline =
		renderer->pipeline;
	uint8_t graphicsAPI = getWindowGraphicsAPI(
		getPipelineWindow(pipeline));

	Matrix4F proj;

	if (camera.perspective.type == PERSPECTIVE_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = createVkPerspectiveMat4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = createGlPerspectiveMat4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
		}
		else
		{
			abort();
		}
	}
	else if (camera.orthographic.type == ORTHOGRAPHIC_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = createVkOrthographicMat4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = createGlOrthographicMat4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
		}
		else
		{
			abort();
		}
	}
	else
	{
		abort();
	}

	Matrix4F view = getTransformModel(
		renderer->parent);
	Matrix4F viewProj = dotMat4F(
		proj,
		view);

	bindPipelineCommand(pipeline);

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];

		if (render->draw == false)
			continue;

		Render* parent =
			render->parent;

		while (parent != NULL)
		{
			if (parent->draw == false)
				goto CONTINUE;

			parent = parent->parent;
		}

		Matrix4F model = getTransformModel(
			render->transform);

		Matrix4F mvp = dotMat4F(
			viewProj,
			model);

		render->renderFunction(
			render,
			pipeline,
			&model,
			&view,
			&proj,
			&viewProj,
			&mvp);

	CONTINUE:
		continue;
	}
}

Render* createRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle)
{
	assert(renderer != NULL);
	assert(rotationType < ROTATION_TYPE_COUNT);
	assert(destroyFunction != NULL);
	assert(renderFunction != NULL);

#ifndef NDEBUG
	if (parent != NULL)
		assert(renderer == parent->renderer);
#endif

	Render* render = malloc(
		sizeof(Render));

	if (render == NULL)
		return NULL;

	Transform* transformParent;

	if (parent != NULL)
	{
		assert(renderer == parent->renderer);
		transformParent = parent->transform;
	}
	else
	{
		transformParent = NULL;
	}

	Transform* transform = createTransform(
		renderer->transformer,
		position,
		scale,
		rotation,
		rotationType,
		transformParent);

	if (transform == NULL)
	{
		free(render);
		return NULL;
	}

	render->renderer = renderer;
	render->draw = draw;
	render->transform = transform;
	render->parent = parent;
	render->destroyFunction = destroyFunction;
	render->renderFunction = renderFunction;
	render->handle = handle;

	if (renderer->renderCount ==
		renderer->renderCapacity)
	{
		size_t capacity =
			renderer->renderCapacity * 2;
		Render** renders = realloc(
			renderer->renders,
			capacity * sizeof(Render*));

		if (renders == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renders = renders;
		renderer->renderCapacity = capacity;
	}

	renderer->renders[
		renderer->renderCount] = render;
	renderer->renderCount++;
	return render;
}
void destroyRender(
	Render* render)
{
	if (render == NULL)
		return;

	Renderer* renderer =
		render->renderer;
	size_t renderCount =
		renderer->renderCount;
	Render** renders =
		renderer->renders;

	for (size_t i = 0; i < renderCount; i++)
	{
		if (renders[i] == render)
		{
			for (size_t j = i + 1; j < renderCount; j++)
				renders[j - 1] = renders[j];

			render->destroyFunction(render->handle);
			destroyTransform(render->transform);
			free(render);

			renderer->renderCount--;
			return;
		}
	}

	abort();
}

Renderer* getRenderRenderer(
	const Render* render)
{
	assert(render != NULL);
	return render->renderer;
}
Transform* getRenderTransform(
	const Render* render)
{
	assert(render != NULL);
	return render->transform;
}
void* getRenderHandle(
	const Render* render)
{
	assert(render != NULL);
	return render->handle;
}

bool getRenderDraw(
	const Render* render)
{
	assert(render != NULL);
	return render->draw;
}
void setRenderDraw(
	Render* render,
	bool value)
{
	assert(render != NULL);
	render->draw = value;
}

Vector3F getRenderPosition(
	const Render* render)
{
	assert(render != NULL);

	return getTransformPosition(
		render->transform);
}
void setRenderPosition(
	Render* render,
	Vector3F position)
{
	assert(render != NULL);

	setTransformPosition(
		render->transform,
		position);
}

Vector3F getRenderScale(
	const Render* render)
{
	assert(render != NULL);

	return getTransformScale(
		render->transform);
}
void setRenderScale(
	Render* render,
	Vector3F scale)
{
	assert(render != NULL);

	setTransformScale(
		render->transform,
		scale);
}

Quaternion getRenderRotation(
	const Render* render)
{
	assert(render != NULL);

	return getTransformRotation(
		render->transform);
}
void setRenderRotation(
	Render* render,
	Quaternion rotation)
{
	assert(render != NULL);

	setTransformRotation(
		render->transform,
		rotation);
}

Render* getRenderParent(
	const Render* render)
{
	assert(render != NULL);
	return render->parent;
}
void setRenderParent(
	Render* render,
	Render* parent)
{
	assert(render != NULL);
	render->parent = parent;

	if (parent != NULL)
	{
		assert(render->renderer ==
			parent->renderer);

		setTransformParent(
			render->transform,
			parent->transform);
	}
	else
	{
		setTransformParent(
			render->transform,
			NULL);
	}
}
