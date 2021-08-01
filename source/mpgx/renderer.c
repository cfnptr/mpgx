#include "mpgx/renderer.h"

#include <assert.h>
#include <string.h>

struct Render
{
	Renderer renderer;
	Transform transform;
	Box3F bounding;
	void* handle;
};
typedef struct RenderElement
{
	Vec3F rendererPosition;
	Vec3F renderPosition;
	Render render;
} RenderElement;
struct Renderer
{
	Transform transform;
	Pipeline pipeline;
	uint8_t sortingType;
	bool useCulling;
	OnRenderDestroy onDestroy;
	OnRenderDraw onDraw;
	Render* renders;
	RenderElement* renderElements;
	size_t renderCapacity;
	size_t renderCount;
};

Renderer createRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	OnRenderDestroy onDestroy,
	OnRenderDraw onDraw,
	size_t capacity)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);
	assert(onDestroy != NULL);
	assert(onDraw != NULL);
	assert(capacity != 0);

	Renderer renderer = malloc(
		sizeof(struct Renderer));

	if (renderer == NULL)
		return NULL;

	Render* renders = malloc(
		sizeof(Render) * capacity);

	if (renders == NULL)
	{
		free(renderer);
		return NULL;
	}

	RenderElement* renderElements = malloc(
		sizeof(RenderElement) * capacity);

	if (renderElements == NULL)
	{
		free(renders);
		free(renderer);
		return NULL;
	}

	renderer->transform = transform;
	renderer->pipeline = pipeline;
	renderer->sortingType = sortingType;
	renderer->useCulling = useCulling;
	renderer->onDestroy = onDestroy;
	renderer->onDraw = onDraw;
	renderer->renders = renders;
	renderer->renderElements = renderElements;
	renderer->renderCapacity = capacity;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(Renderer renderer)
{
	if (renderer == NULL)
		return;

	free(renderer->renderElements);

	Render* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;
	OnRenderDestroy onDestroy = renderer->onDestroy;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render render = renders[i];
		onDestroy(render->handle);
		free(render);
	}

	free(renders);
	free(renderer);
}

bool isRendererEmpty(Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->renderCount == 0;
}
Transform getRendererTransform(Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->transform;
}
Pipeline getRendererPipeline(Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->pipeline;
}
OnRenderDestroy getRendererOnDestroy(Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->onDestroy;
}
OnRenderDraw getRendererOnDraw(Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->onDraw;
}

uint8_t getRendererSortingType(
	Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->sortingType;
}
void setRendererSortingType(
	Renderer renderer,
	uint8_t sortingType)
{
	assert(renderer != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);
	renderer->sortingType = sortingType;
}

bool getRendererUseCulling(
	Renderer renderer)
{
	assert(renderer != NULL);
	return renderer->useCulling;
}
void setRendererUseCulling(
	Renderer renderer,
	bool useCulling)
{
	assert(renderer != NULL);
	renderer->useCulling = useCulling;
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
	Window window,
	Mat4F view,
	Camera camera,
	RenderData* data)
{
	assert(window != NULL);
	assert(data != NULL);

	uint8_t graphicsAPI =
		getWindowGraphicsAPI(window);

	Mat4F proj;
	Mat4F viewProj;

	if (camera.persp.type == PERSP_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = perspZeroOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			frustumZeroOneMat4F(
				viewProj,
				&data->leftPlane,
				&data->rightPlane,
				&data->bottomPlane,
				&data->topPlane,
				&data->backPlane,
				&data->frontPlane,
				false);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = perspNegOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			frustumNegOneMat4F(
				viewProj,
				&data->leftPlane,
				&data->rightPlane,
				&data->bottomPlane,
				&data->topPlane,
				&data->backPlane,
				&data->frontPlane,
				false);
		}
		else
		{
			abort();
		}
	}
	else if (camera.ortho.type == ORTHO_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = orthoZeroOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			frustumZeroOneMat4F(
				viewProj,
				&data->leftPlane,
				&data->rightPlane,
				&data->bottomPlane,
				&data->topPlane,
				&data->backPlane,
				&data->frontPlane,
				false);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = orthoNegOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj,
				view);
			frustumNegOneMat4F(
				viewProj,
				&data->leftPlane,
				&data->rightPlane,
				&data->bottomPlane,
				&data->topPlane,
				&data->backPlane,
				&data->frontPlane,
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

	data->proj = proj;
	data->viewProj = viewProj;
}
void drawRenderer(
	Renderer renderer,
	const RenderData* data)
{
	assert(renderer != NULL);
	assert(data != NULL);

	size_t renderCount = renderer->renderCount;

	if (renderCount == 0)
		return;

	Render* renders = renderer->renders;
	RenderElement* renderElements = renderer->renderElements;
	bool useCulling = renderer->useCulling;

	Vec3F rendererPosition = negVec3F(
		getTransformPosition(renderer->transform));

	Plane3F leftPlane = data->leftPlane;
	Plane3F rightPlane = data->rightPlane;
	Plane3F bottomPlane = data->bottomPlane;
	Plane3F topPlane = data->topPlane;
	Plane3F backPlane = data->backPlane;
	Plane3F frontPlane = data->frontPlane;

	size_t renderElementCount = 0;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render render = renders[i];
		Transform transform = render->transform;

		if (isTransformActive(transform) == false)
			continue;

		Transform parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (isTransformActive(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Mat4F model = getTransformModel(transform);
		Vec3F renderPosition = getTranslationMat4F(model);

		if (useCulling == true)
		{
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

			bool isInFrustum = isBoxInFrustum(
				leftPlane,
				rightPlane,
				bottomPlane,
				topPlane,
				backPlane,
				frontPlane,
				renderBounding);

			if (isInFrustum == false)
				continue;
		}

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

	if (renderElementCount > 1)
	{
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
		else
		{
			abort();
		}
	}

	Mat4F viewProj = data->viewProj;
	Pipeline pipeline = renderer->pipeline;
	OnRenderDraw onDraw = renderer->onDraw;

	bindPipeline(pipeline);

	for (size_t i = 0; i < renderElementCount; i++)
	{
		Render render = renderElements[i].render;

		Mat4F model = getTransformModel(
			render->transform);

		onDraw(
			render,
			pipeline,
			&model,
			&viewProj);
	}
}

Render createRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	void* handle)
{
	assert(renderer != NULL);
	assert(transform != NULL);

	assert(getTransformTransformer(transform) ==
		getTransformTransformer(renderer->transform));

	Render render = malloc(
		sizeof(struct Render));

	if (render == NULL)
		return NULL;

	render->renderer = renderer;
	render->transform = transform;
	render->bounding = bounding;
	render->handle = handle;

	size_t count = renderer->renderCount;

	if (count == renderer->renderCapacity)
	{
		size_t capacity = renderer->renderCapacity * 2;

		Render* renders = realloc(
			renderer->renders,
			sizeof(Render) * capacity);

		if (renders == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renders = renders;

		RenderElement* renderElements = realloc(
			renderer->renderElements,
			sizeof(RenderElement) * capacity);

		if (renderElements == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renderElements = renderElements;
		renderer->renderCapacity = capacity;
	}

	renderer->renders[count] = render;
	renderer->renderCount = count + 1;
	return render;
}
void destroyRender(Render render)
{
	if (render == NULL)
		return;

	Renderer renderer = render->renderer;
	Render* renders = renderer->renders;
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

Renderer getRenderRenderer(Render render)
{
	assert(render != NULL);
	return render->renderer;
}
Transform getRenderTransform(Render render)
{
	assert(render != NULL);
	return render->transform;
}

Box3F getRenderBounding(
	Render render)
{
	assert(render != NULL);
	return render->bounding;
}
void setRenderBounding(
	Render render,
	Box3F bounding)
{
	assert(render != NULL);
	render->bounding = bounding;
}

void* getRenderHandle(Render render)
{
	assert(render != NULL);
	return render->handle;
}
