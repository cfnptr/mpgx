#include "mpgx/renderer.h"

#include <assert.h>
#include <string.h>

struct Render
{
	Renderer* renderer;
	Transform* transform;
	Box3F bounding;
	void* handle;
};
typedef struct RenderElement
{
	Vec3F rendererPosition;
	Vec3F renderPosition;
	Render* render;
} RenderElement;
struct Renderer
{
	Transform* transform;
	Pipeline* pipeline;
	uint8_t sortingType;
	OnRenderDestroy onDestroy;
	OnRenderDraw onDraw;
	Render** renders;
	RenderElement* renderElements;
	size_t renderCapacity;
	size_t renderCount;
};

Renderer* createRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType,
	OnRenderDestroy onDestroy,
	OnRenderDraw onDraw)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);
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

	RenderElement* renderElements = malloc(
		sizeof(RenderElement));

	if (renderElements == NULL)
	{
		free(renders);
		free(renderer);
		return NULL;
	}

	renderer->transform = transform;
	renderer->pipeline = pipeline;
	renderer->sortingType = sortingType;
	renderer->onDestroy = onDestroy;
	renderer->onDraw = onDraw;
	renderer->renders = renders;
	renderer->renderElements = renderElements;
	renderer->renderCapacity = 1;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(Renderer* renderer)
{
	if (renderer == NULL)
		return;

	free(renderer->renderElements);

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

uint8_t getRendererSorting(
	const Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->sortingType;
}
void setRendererSorting(
	Renderer* renderer,
	uint8_t sortingType)
{
	assert(renderer != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);
	renderer->sortingType = sortingType;
}

static int ascendingRenderCompare(
	const void* a,
	const void* b)
{
	const RenderElement* data = (RenderElement*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (RenderElement*)b;

	float distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	if (distanceA < distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA > distanceB)
		return 1;

	abort();
}
static int descendingRenderCompare(
	const void* a,
	const void* b)
{
	const RenderElement* data = (RenderElement*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (RenderElement*)b;

	float distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	if (distanceA > distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA < distanceB)
		return 1;

	abort();
}

void createRenderData(
	const Window* window,
	Mat4F view,
	Camera camera,
	RenderData* data)
{
	assert(window != NULL);
	assert(data != NULL);

	uint8_t graphicsAPI = getWindowGraphicsAPI(window);

	Mat4F proj;
	Mat4F viewProj;

	if (camera.perspective.type == PERSPECTIVE_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = vkPerspectiveMat4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			vkFrustumPlanes(
				viewProj,
				data->planes,
				false);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = glPerspectiveMat4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			glFrustumPlanes(
				viewProj,
				data->planes,
				false);
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
			proj = vkOrthographicMat4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			vkFrustumPlanes(
				viewProj,
				data->planes,
				false);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = glOrthographicMat4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			glFrustumPlanes(
				viewProj,
				data->planes,
				false);
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

	data->view = view;
	data->proj = proj;
	data->viewProj = viewProj;
}
void drawRenderer(
	Renderer* renderer,
	const RenderData* data)
{
	assert(renderer != NULL);

	Render** renders = renderer->renders;
	RenderElement* renderElements = renderer->renderElements;
	size_t renderCount = renderer->renderCount;

	Vec3F rendererPosition = negVec3F(
		getTransformPosition(renderer->transform));
	const Plane3F* planes = data->planes;

	size_t renderElementCount = 0;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];
		Transform* transform = render->transform;

		if (isTransformActive(transform) == false)
			continue;

		Transform* parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (isTransformActive(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Mat4F model = getTransformModel(transform);
		Vec3F renderPosition = getTranslationMat4F(model);
		Vec3F renderScale = getScaleMat4F(model);
		Box3F renderBounding = getRenderBounding(render);

		renderBounding.minimum = mulVec3F(
			renderBounding.minimum,
			renderScale);
		renderBounding.minimum = addVec3F(
			renderBounding.minimum,
			renderPosition);
		renderBounding.maximum = mulVec3F(
			renderBounding.maximum,
			renderScale);
		renderBounding.maximum = addVec3F(
			renderBounding.maximum,
			renderPosition);

		if (isBoxInFrustum(planes, renderBounding) == false)
			continue;

		RenderElement element;
		element.rendererPosition = rendererPosition;
		element.renderPosition = renderPosition;
		element.render = render;
		renderElements[renderElementCount++] = element;

	CONTINUE:
		continue;
	}

	if (renderElementCount == 0)
		return;

	uint8_t sortingType = renderer->sortingType;

	if (sortingType == ASCENDING_RENDER_SORTING)
	{
		qsort(
			renderElements,
			renderElementCount,
			sizeof(RenderElement),
			ascendingRenderCompare);
	}
	else if (sortingType == DESCENDING_RENDER_SORTING)
	{
		qsort(
			renderElements,
			renderElementCount,
			sizeof(RenderElement),
			descendingRenderCompare);
	}

	Mat4F view = data->view;
	Mat4F proj = data->proj;
	Mat4F viewProj = data->viewProj;
	Pipeline* pipeline = renderer->pipeline;
	OnRenderDraw onDraw = renderer->onDraw;

	bindPipeline(pipeline);

	for (size_t i = 0; i < renderElementCount; i++)
	{
		Render* render = renderElements[i].render;

		Mat4F model = getTransformModel(
			render->transform);
		Mat4F mvp = dotMat4F(
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
	}
}

Render* createRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
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
	render->bounding = bounding;
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

		RenderElement* renderElements = realloc(
			renderer->renderElements,
			renderCapacity * sizeof(RenderElement));

		if (renderElements == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renderElements = renderElements;
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

Box3F getRenderBounding(
	const Render* render)
{
	assert(render != NULL);
	return render->bounding;
}
void setRenderBounding(
	Render* render,
	Box3F bounding)
{
	assert(render != NULL);
	render->bounding = bounding;
}

void* getRenderHandle(const Render* render)
{
	assert(render != NULL);
	return render->handle;
}
