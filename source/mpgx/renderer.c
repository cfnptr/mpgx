#include "mpgx/renderer.h"

#include <assert.h>
#include <string.h>

struct Renderer
{
	Transform* transform;
	Pipeline* pipeline;
	bool ascendingSort;
	OnRenderDestroy onDestroy;
	OnRenderDraw onDraw;
	Render** renders;
	size_t renderCapacity;
	size_t renderCount;
};
struct Render
{
	Renderer* renderer;
	Transform* transform;
	void* handle;
};

static int ascendCompareRender(
	const void* a,
	const void* b)
{
	Render* render = *(Render**)a;

	Vector3F renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceA = distPowVec3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render = *(Render**)b;

	renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceB = distPowVec3F(
		getTransformPosition(render->renderer->transform),
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
	Render* render = *(Render**)a;

	Vector3F renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceA = distPowVec3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render = *(Render**)b;

	renderPosition = addVec3F(
		getTransformPosition(render->transform),
		getTranslationMat4F(getTransformModel(render->transform)));
	float distanceB = distPowVec3F(
		getTransformPosition(render->renderer->transform),
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
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSort,
	OnRenderDestroy onDestroy,
	OnRenderDraw onDraw)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(onDestroy != NULL);
	assert(onDraw != NULL);

	Renderer* renderer = malloc(sizeof(Renderer));

	if (renderer == NULL)
		return NULL;

	Render** renders = malloc(sizeof(Render*));

	if (renders == NULL)
	{
		free(renderer);
		return NULL;
	}

	renderer->transform = transform;
	renderer->pipeline = pipeline;
	renderer->ascendingSort = ascendingSort;
	renderer->onDestroy = onDestroy;
	renderer->onDraw = onDraw;
	renderer->renders = renders;
	renderer->renderCapacity = 1;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(Renderer* renderer)
{
	if (renderer == NULL)
		return;

	Render** renders = renderer->renders;
	size_t renderCount = renderer->renderCount;
	OnRenderDestroy onDestroy = renderer->onDestroy;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];
		onDestroy(render->handle);
		free(render);
	}

	free(renders);
	free(renderer);
}

Transform* getRendererTransform(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->transform;
}
Pipeline* getRendererPipeline(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->pipeline;
}
OnRenderDestroy getRendererOnDestroy(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->onDestroy;
}
OnRenderDraw getRendererOnDraw(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->onDraw;
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

void updateRenderer(
	Renderer* renderer,
	Camera camera)
{
	assert(renderer != NULL);

	Render** renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

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
		renderer->transform);
	Matrix4F viewProj = dotMat4F(
		proj,
		view);

	OnRenderDraw onDraw =
		renderer->onDraw;

	bindPipeline(pipeline);

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];

		if (getTransformUpdate(render->transform) == false)
			continue;

		Transform* parent = getTransformParent(
			render->transform);

		while (parent != NULL)
		{
			if (getTransformUpdate(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Matrix4F model = getTransformModel(
			render->transform);
		Matrix4F mvp = dotMat4F(
			viewProj,
			model);

		onDraw(
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
	Transform* transform,
	void* handle)
{
	assert(renderer != NULL);
	assert(transform != NULL);

	assert(getTransformTransformer(transform) ==
		getTransformTransformer(renderer->transform));

	Render* render = malloc(sizeof(Render));

	if (render == NULL)
		return NULL;

	render->renderer = renderer;
	render->transform = transform;
	render->handle = handle;

	Render** renders = renderer->renders;
	size_t renderCount = renderer->renderCount;
	size_t renderCapacity = renderer->renderCapacity;

	if (renderCount == renderCapacity)
	{
		renderCapacity *= 2;

		renders = realloc(
			renders,
			renderCapacity * sizeof(Render*));

		if (renders == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renders = renders;
		renderer->renderCapacity = renderCapacity;
	}

	renders[renderCount] = render;
	renderer->renderCount++;
	return render;
}
void destroyRender(Render* render)
{
	if (render == NULL)
		return;

	Renderer* renderer = render->renderer;
	Render** renders = renderer->renders;
	size_t renderCount = renderer->renderCount;
	OnRenderDestroy onDestroy = renderer->onDestroy;

	for (size_t i = 0; i < renderCount; i++)
	{
		if (renders[i] != render)
			continue;

		for (size_t j = i + 1; j < renderCount; j++)
			renders[j - 1] = renders[j];

		onDestroy(render->handle);
		free(render);

		renderer->renderCount--;
		return;
	}

	abort();
}

Renderer* getRenderRenderer(const Render* render)
{
	assert(render != NULL);
	return render->renderer;
}
Transform* getRenderTransform(const Render* render)
{
	assert(render != NULL);
	return render->transform;
}
void* getRenderHandle(const Render* render)
{
	assert(render != NULL);
	return render->handle;
}
