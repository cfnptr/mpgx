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

#include "mpgx/renderers/texture_sprite_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
	Vec2F size;
	Vec2F offset;
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
	assert(graphicsRender != NULL);
	assert(graphicsPipeline != NULL);
	assert(model != NULL);
	assert(viewProj != NULL);

	Handle handle = getGraphicsRenderHandle(
		graphicsRender);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTextureSpritePipelineMvp(
		graphicsPipeline,
		mvp);
	setTextureSpritePipelineColor(
		graphicsPipeline,
		handle->color);
	setTextureSpritePipelineSize(
		graphicsPipeline,
		handle->size);
	setTextureSpritePipelineOffset(
		graphicsPipeline,
		handle->offset);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createTextureSpriteRenderer(
	GraphicsPipeline textureSpritePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(textureSpritePipeline != NULL);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(getGraphicsPipelineName(
		textureSpritePipeline),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		textureSpritePipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity);
}
GraphicsRender createTextureSpriteRender(
	GraphicsRenderer textureSpriteRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh)
{
	assert(textureSpriteRenderer != NULL);
	assert(transform != NULL);
	assert(mesh != NULL);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		textureSpriteRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		textureSpriteRenderer)),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

	handle->color = color;
	handle->size = size;
	handle->offset = offset;
	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		textureSpriteRenderer,
		transform,
		bounding,
		handle);

	if (render == NULL)
	{
		onDestroy(handle);
		return NULL;
	}

	return render;
}

LinearColor getTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->color;
}
void setTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender,
	LinearColor color)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->color = color;
}

Vec2F getTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->size;
}
void setTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender,
	Vec2F size)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->size = size;
}

Vec2F getTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->offset;
}
void setTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender,
	Vec2F offset)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->offset = offset;
}

GraphicsMesh getTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->mesh;
}
void setTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender,
	GraphicsMesh mesh)
{
	assert(textureSpriteRender != NULL);
	assert(mesh != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->mesh = mesh;
}
