// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
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

#include "mpgx/renderers/texture_color_renderer.h"

#include <string.h>
#include <assert.h>

struct RenderHandle_T
{
	LinearColor color;
	Vec2F size;
	Vec2F offset;
	Mesh mesh;
};

typedef struct RenderHandle_T RenderHandle_T;
typedef RenderHandle_T* RenderHandle;

static void onDestroy(void* handle)
{
	free((RenderHandle)handle);
}
static size_t onDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	RenderHandle renderHandle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTextureColorPipelineMvp(
		pipeline,
		mvp);
	setTextureColorPipelineColor(
		pipeline,
		renderHandle->color);
	setTextureColorPipelineSize(
		pipeline,
		renderHandle->size);
	setTextureColorPipelineOffset(
		pipeline,
		renderHandle->offset);
	return drawMesh(
		renderHandle->mesh,
		pipeline);
}
Renderer createTextureSpriteRenderer(
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(pipeline != NULL);
	assert(sorting < RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(
		getPipelineName(pipeline),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);

	return createRenderer(
		pipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity);
}
Render createTextureColorRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assert(mesh != NULL);

	assert(getFramebufferWindow(
		getPipelineFramebuffer(
		getRendererPipeline(renderer))) ==
		getMeshWindow(mesh));
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);

	RenderHandle renderHandle = malloc(
		sizeof(RenderHandle_T));

	if (renderHandle == NULL)
		return NULL;

	renderHandle->color = color;
	renderHandle->size = size;
	renderHandle->offset = offset;
	renderHandle->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		renderHandle);

	if (render == NULL)
	{
		free(renderHandle);
		return NULL;
	}

	return render;
}

LinearColor getTexColRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	return renderHandle->color;
}
void setTexColRenderColor(
	Render render,
	LinearColor color)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	renderHandle->color = color;
}

Vec2F getTexColRenderSize(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	return renderHandle->size;
}
void setTexColRenderSize(
	Render render,
	Vec2F size)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	renderHandle->size = size;
}

Vec2F getTexColRenderOffset(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	return renderHandle->offset;
}
void setTexColRenderOffset(
	Render render,
	Vec2F offset)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	renderHandle->offset = offset;
}

Mesh getTexColRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	return renderHandle->mesh;
}
void setTexColRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	renderHandle->mesh = mesh;
}
