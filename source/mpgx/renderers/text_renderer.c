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

#include "mpgx/renderers/text_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
	Text text;
	Vec4U scissor;
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
	setTextPipelineMVP(
		graphicsPipeline,
		mvp);
	setTextPipelineColor(
		graphicsPipeline,
		handle->color);
	return drawText(
		handle->text,
		handle->scissor);
}
GraphicsRenderer createTextRenderer(
	GraphicsPipeline textPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(textPipeline != NULL);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(getGraphicsPipelineName(
		textPipeline),
		TEXT_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		textPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity);
}
GraphicsRender createTextRender(
	GraphicsRenderer textRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Text text,
	Vec4U scissor)
{
	assert(textRenderer != NULL);
	assert(transform != NULL);
	assert(text != NULL);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		textRenderer)) ==
		getGraphicsPipelineWindow(
		getTextPipeline(text)));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		textRenderer)),
		TEXT_PIPELINE_NAME) == 0);

	Handle handle = malloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

	handle->color = color;
	handle->text = text;
	handle->scissor = scissor;

	GraphicsRender render = createGraphicsRender(
		textRenderer,
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

LinearColor getTextRenderColor(
	GraphicsRender textRender)
{
	assert(textRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	return handle->color;
}
void setTextRenderColor(
	GraphicsRender textRender,
	LinearColor color)
{
	assert(textRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	handle->color = color;
}

Text getTextRenderText(
	GraphicsRender textRender)
{
	assert(textRender != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	return handle->text;
}
void setTextRenderText(
	GraphicsRender textRender,
	Text text)
{
	assert(textRender != NULL);
	assert(text != NULL);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	handle->text = text;
}
