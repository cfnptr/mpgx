// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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

#include "mpgx/graphics_renderer.h"

#include <assert.h>
#include <string.h>

struct GraphicsRender_T
{
	GraphicsRenderer renderer;
	Transform transform;
	Box3F bounding;
	void* handle;
};
typedef struct GraphicsRenderElement
{
	Vec3F rendererPosition;
	Vec3F renderPosition;
	GraphicsRender render;
} GraphicsRenderElement;
struct GraphicsRenderer_T
{
	GraphicsPipeline pipeline;
	GraphicsRenderSorting sorting;
	bool useCulling;
	OnGraphicsRenderDestroy onDestroy;
	OnGraphicsRenderDraw onDraw;
	GraphicsRender* renders;
	GraphicsRenderElement* renderElements;
	size_t renderCapacity;
	size_t renderCount;
};

void destroyGraphicsRenderer(
	GraphicsRenderer graphicsRenderer)
{
	if (graphicsRenderer == NULL)
		return;

	assert(graphicsRenderer->renderCount == 0);

	free(graphicsRenderer->renderElements);
	free(graphicsRenderer->renders);
	free(graphicsRenderer);
}
GraphicsRenderer createGraphicsRenderer(
	GraphicsPipeline pipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	OnGraphicsRenderDestroy onDestroy,
	OnGraphicsRenderDraw onDraw,
	size_t capacity)
{
	assert(pipeline != NULL);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(onDestroy != NULL);
	assert(onDraw != NULL);
	assert(capacity != 0);

	GraphicsRenderer graphicsRenderer = calloc(1,
		sizeof(GraphicsRenderer_T));

	if (graphicsRenderer == NULL)
		return NULL;

	graphicsRenderer->pipeline = pipeline;
	graphicsRenderer->sorting = sorting;
	graphicsRenderer->useCulling = useCulling;
	graphicsRenderer->onDestroy = onDestroy;
	graphicsRenderer->onDraw = onDraw;

	GraphicsRender* renders = malloc(
		sizeof(GraphicsRender) * capacity);

	if (renders == NULL)
	{
		destroyGraphicsRenderer(graphicsRenderer);
		return NULL;
	}

	graphicsRenderer->renders = renders;
	graphicsRenderer->renderCapacity = capacity;
	graphicsRenderer->renderCount = 0;

	GraphicsRenderElement* renderElements = malloc(
		sizeof(GraphicsRenderElement) * capacity);

	if (renderElements == NULL)
	{
		destroyGraphicsRenderer(graphicsRenderer);
		return NULL;
	}

	graphicsRenderer->renderElements = renderElements;
	return graphicsRenderer;
}

GraphicsPipeline getGraphicsRendererPipeline(
	GraphicsRenderer renderer)
{
	assert(renderer != NULL);
	return renderer->pipeline;
}
OnGraphicsRenderDestroy getGraphicsRendererOnDestroy(
	GraphicsRenderer renderer)
{
	assert(renderer != NULL);
	return renderer->onDestroy;
}
OnGraphicsRenderDraw getGraphicsRendererOnDraw(
	GraphicsRenderer renderer)
{
	assert(renderer != NULL);
	return renderer->onDraw;
}

GraphicsRenderSorting getGraphicsRendererSorting(
	GraphicsRenderer renderer)
{
	assert(renderer != NULL);
	return renderer->sorting;
}
void setGraphicsRendererSorting(
	GraphicsRenderer renderer,
	GraphicsRenderSorting sorting)
{
	assert(renderer != NULL);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	renderer->sorting = sorting;
}

bool getGraphicsRendererUseCulling(
	GraphicsRenderer renderer)
{
	assert(renderer != NULL);
	return renderer->useCulling;
}
void setGraphicsRendererUseCulling(
	GraphicsRenderer renderer,
	bool useCulling)
{
	assert(renderer != NULL);
	renderer->useCulling = useCulling;
}

void enumerateGraphicsRenderer(
	GraphicsRenderer renderer,
	void(*onItem)(GraphicsRender))
{
	assert(renderer != NULL);
	assert(onItem != NULL);

	for (size_t i = 0; i < renderer->renderCount; i++)
		onItem(renderer->renders[i]);
}

static int ascendingRenderCompare(
	const void* a,
	const void* b)
{
	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

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
	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

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

void createGraphicsRenderData(
	Window window,
	Mat4F view,
	Camera camera,
	GraphicsRendererData* graphicsRendererData,
	bool createPlanes)
{
	assert(window != NULL);
	assert(graphicsRendererData != NULL);

	GraphicsAPI api = getWindowGraphicsAPI(window);

	Mat4F proj;
	Mat4F viewProj;

	if (camera.persp.type == PERSP_CAMERA_TYPE)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
			proj = perspZeroOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes == true)
			{
				frustumZeroOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			proj = perspNegOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes == true)
			{
				frustumNegOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else
		{
			abort();
		}
	}
	else if (camera.ortho.type == ORTHO_CAMERA_TYPE)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
			proj = orthoZeroOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes == true)
			{
				frustumZeroOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			proj = orthoNegOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes == true)
			{
				frustumNegOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
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

	graphicsRendererData->view = view;
	graphicsRendererData->proj = proj;
	graphicsRendererData->viewProj = viewProj;

	if (createPlanes == false)
	{
		graphicsRendererData->leftPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->rightPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->bottomPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->topPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->backPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->frontPlane = plane3F(zeroVec3F, 0.0f);
	}
}

GraphicsRenderResult drawGraphicsRenderer(
	GraphicsRenderer graphicsRenderer,
	const GraphicsRendererData* graphicsRendererData)
{
	assert(graphicsRenderer != NULL);
	assert(graphicsRendererData != NULL);

	GraphicsRenderResult result;
	result.renderCount = 0;
	result.indexCount = 0;
	result.passCount = 0;

	size_t renderCount = graphicsRenderer->renderCount;

	if (renderCount == 0)
		return result;

	GraphicsRender* renders = graphicsRenderer->renders;
	GraphicsRenderElement* renderElements = graphicsRenderer->renderElements;
	bool useCulling = graphicsRenderer->useCulling;

	Vec3F rendererPosition = negVec3F(
		getTranslationMat4F(graphicsRendererData->view));

	Plane3F leftPlane = graphicsRendererData->leftPlane;
	Plane3F rightPlane = graphicsRendererData->rightPlane;
	Plane3F bottomPlane = graphicsRendererData->bottomPlane;
	Plane3F topPlane = graphicsRendererData->topPlane;
	Plane3F backPlane = graphicsRendererData->backPlane;
	Plane3F frontPlane = graphicsRendererData->frontPlane;

	size_t elementCount = 0;

	for (size_t i = 0; i < renderCount; i++)
	{
		GraphicsRender render = renders[i];
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
			Vec3F renderScale = getTransformScale(transform);
			Box3F renderBounding = getGraphicsRenderBounding(render);

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

		GraphicsRenderElement element;
		element.rendererPosition = rendererPosition;
		element.renderPosition = renderPosition;
		element.render = render;
		renderElements[elementCount++] = element;

	CONTINUE:
		continue;
	}

	if (elementCount == 0)
		return result;

	GraphicsRenderSorting sorting = graphicsRenderer->sorting;

	if (sorting != NO_GRAPHICS_RENDER_SORTING && elementCount > 1)
	{
		if (sorting == ASCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				ascendingRenderCompare);
		}
		else if (sorting == DESCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				descendingRenderCompare);
		}
		else
		{
			abort();
		}
	}

	Mat4F viewProj = graphicsRendererData->viewProj;
	GraphicsPipeline pipeline = graphicsRenderer->pipeline;
	OnGraphicsRenderDraw onDraw = graphicsRenderer->onDraw;

	bindGraphicsPipeline(pipeline);

	for (size_t i = 0; i < elementCount; i++)
	{
		GraphicsRender render = renderElements[i].render;

		Mat4F model = getTransformModel(
			render->transform);

		size_t indexCount = onDraw(
			render,
			pipeline,
			&model,
			&viewProj);

		if (indexCount != 0)
		{
			result.renderCount++;
			result.indexCount += indexCount;
		}
	}

	return result;
}

GraphicsRender createGraphicsRender(
	GraphicsRenderer renderer,
	Transform transform,
	Box3F bounding,
	void* handle)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assert(handle != NULL);

	GraphicsRender graphicsRender = malloc(
		sizeof(GraphicsRender_T));

	if (graphicsRender == NULL)
		return NULL;

	graphicsRender->renderer = renderer;
	graphicsRender->transform = transform;
	graphicsRender->bounding = bounding;
	graphicsRender->handle = handle;

	size_t count = renderer->renderCount;

	if (count == renderer->renderCapacity)
	{
		size_t capacity = renderer->renderCapacity * 2;

		GraphicsRender* renders = realloc(
			renderer->renders,
			sizeof(GraphicsRender) * capacity);

		if (renders == NULL)
		{
			free(graphicsRender);
			return NULL;
		}

		renderer->renders = renders;

		GraphicsRenderElement* renderElements = realloc(
			renderer->renderElements,
			sizeof(GraphicsRenderElement) * capacity);

		if (renderElements == NULL)
		{
			free(graphicsRender);
			return NULL;
		}

		renderer->renderElements = renderElements;
		renderer->renderCapacity = capacity;
	}

	renderer->renders[count] = graphicsRender;
	renderer->renderCount = count + 1;
	return graphicsRender;
}
void destroyGraphicsRender(
	GraphicsRender graphicsRender,
	bool _destroyTransform)
{
	if (graphicsRender == NULL)
		return;

	GraphicsRenderer renderer = graphicsRender->renderer;
	GraphicsRender* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

	for (size_t i = 0; i < renderCount; i++)
	{
		if (renders[i] != graphicsRender)
			continue;

		for (size_t j = i + 1; j < renderCount; j++)
			renders[j - 1] = renders[j];

		renderer->onDestroy(graphicsRender->handle);

		if (_destroyTransform == true)
			destroyTransform(graphicsRender->transform);

		free(graphicsRender);
		renderer->renderCount--;
		return;
	}

	abort();
}

GraphicsRenderer getGraphicsRenderRenderer(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender != NULL);
	return graphicsRender->renderer;
}
Transform getGraphicsRenderTransform(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender != NULL);
	return graphicsRender->transform;
}

Box3F getGraphicsRenderBounding(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender != NULL);
	return graphicsRender->bounding;
}
void setGraphicsRenderBounding(
	GraphicsRender graphicsRender,
	Box3F bounding)
{
	assert(graphicsRender != NULL);
	graphicsRender->bounding = bounding;
}

void* getGraphicsRenderHandle(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender != NULL);
	return graphicsRender->handle;
}
