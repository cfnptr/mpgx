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
typedef struct RenderData
{
	Vec3F rendererPosition;
	Vec3F renderPosition;
	Render* render;
} RenderData;
struct Renderer
{
	Transform* transform;
	Pipeline* pipeline;
	bool ascendingSort;
	OnRenderDestroy onDestroy;
	OnRenderDraw onDraw;
	Render** renders;
	RenderData* renderData;
	size_t renderCapacity;
	size_t renderCount;
};

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

	RenderData* renderData = malloc(sizeof(RenderData));

	if (renderData == NULL)
	{
		free(renders);
		free(renderer);
		return NULL;
	}

	renderer->transform = transform;
	renderer->pipeline = pipeline;
	renderer->ascendingSort = ascendingSort;
	renderer->onDestroy = onDestroy;
	renderer->onDraw = onDraw;
	renderer->renders = renders;
	renderer->renderData = renderData;
	renderer->renderCapacity = 1;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(Renderer* renderer)
{
	if (renderer == NULL)
		return;

	free(renderer->renderData);

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

static int ascendCompareRender(
	const void* a,
	const void* b)
{
	const RenderData* data = (RenderData*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (RenderData*)b;

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
static int descendCompareRender(
	const void* a,
	const void* b)
{
	const RenderData* data = (RenderData*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (RenderData*)b;

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

void updateRenderer(
	Renderer* renderer,
	Camera camera)
{
	assert(renderer != NULL);

	Pipeline* pipeline = renderer->pipeline;
	Mat4F view = getTransformModel(renderer->transform);

	uint8_t graphicsAPI = getWindowGraphicsAPI(
		getPipelineWindow(pipeline));

	Mat4F proj;
	Mat4F viewProj;
	Plane3F planes[6];

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
				planes,
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
				planes,
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
				planes,
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
				planes,
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

	Render** renders = renderer->renders;
	RenderData* renderData = renderer->renderData;
	size_t renderCount = renderer->renderCount;

	Vec3F rendererPosition = getTransformPosition(
		renderer->transform);

	size_t renderDataCount = 0;

	for (size_t i = 0; i < renderCount; i++)
	{
		Render* render = renders[i];
		Transform* transform = render->transform;

		if (getTransformUpdate(transform) == false)
			continue;

		Transform* parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (getTransformUpdate(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Vec3F renderPosition = getTranslationMat4F(
			getTransformModel(transform));
		Box3F renderBounding = getRenderBounding(render);

		renderBounding.minimum = addVec3F(
			renderBounding.minimum,
			renderPosition);
		renderBounding.maximum = addVec3F(
			renderBounding.maximum,
			renderPosition);

		if (isBoxInFrustum(planes, renderBounding) == false)
			continue;

		RenderData data;
		data.rendererPosition = rendererPosition;
		data.renderPosition = renderPosition;
		data.render = render;
		renderData[renderDataCount++] = data;

	CONTINUE:
		continue;
	}

	if (renderDataCount == 0)
		return;

	if (renderer->ascendingSort == true)
	{
		qsort(
			renderData,
			renderDataCount,
			sizeof(RenderData),
			ascendCompareRender);
	}
	else
	{
		qsort(
			renderData,
			renderDataCount,
			sizeof(RenderData),
			descendCompareRender);
	}

	OnRenderDraw onDraw = renderer->onDraw;

	bindPipeline(pipeline);

	for (size_t i = 0; i < renderDataCount; i++)
	{
		Render* render = renderData[i].render;

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

		RenderData* renderData = realloc(
			renderer->renderData,
			renderCapacity * sizeof(RenderData));

		if (renderData == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renderData = renderData;
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
