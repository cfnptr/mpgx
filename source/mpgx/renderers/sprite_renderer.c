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

#include "mpgx/renderers/sprite_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
	GraphicsMesh mesh;
} Handle_T;

typedef Handle_T* Handle;

static void onDestroy(void* handle)
{
	free((Handle)handle);
}
static size_t onDraw(
	GraphicsRender graphicsRender,
	GraphicsPipeline graphicsPipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	Handle handle = getGraphicsRenderHandle(
		graphicsRender);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setSpritePipelineMvp(
		graphicsPipeline,
		mvp);
	setSpritePipelineColor(
		graphicsPipeline,
		handle->color);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createSpriteRenderer(
	GraphicsPipeline spritePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(spritePipeline != NULL);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(getGraphicsPipelineName(
		spritePipeline),
		SPRITE_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		spritePipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity);
}
GraphicsRender createSpriteRender(
	GraphicsRenderer spriteRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	GraphicsMesh mesh)
{
	assert(spriteRenderer != NULL);
	assert(transform != NULL);
	assert(mesh != NULL);

	assert(getFramebufferWindow(
		getGraphicsPipelineFramebuffer(
		getGraphicsRendererPipeline(
		spriteRenderer))) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		spriteRenderer)),
		SPRITE_PIPELINE_NAME) == 0);

	Handle handle = malloc(sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

	handle->color = color;
	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		spriteRenderer,
		transform,
		bounding,
		handle);

	if (render == NULL)
	{
		free(handle);
		return NULL;
	}

	return render;
}

LinearColor getSpriteRenderColor(
	GraphicsRender spriteRender)
{
	assert(spriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->color;
}
void setSpriteRenderColor(
	GraphicsRender spriteRender,
	LinearColor color)
{
	assert(spriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->color = color;
}

GraphicsMesh getSpriteRenderMesh(
	GraphicsRender spriteRender)
{
	assert(spriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->mesh;
}
void setSpriteRenderMesh(
	GraphicsRender spriteRender,
	GraphicsMesh mesh)
{
	assert(spriteRender != NULL);
	assert(mesh != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->mesh = mesh;
}
