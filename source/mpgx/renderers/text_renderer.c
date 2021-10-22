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

#include "mpgx/renderers/text_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct RenderHandle
{
	LinearColor color;
	Text text;
} RenderHandle;

static void onRenderHandleDestroy(void* handle)
{
	free((RenderHandle*)handle);
}
static size_t onRenderHandleDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	RenderHandle* renderHandle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTextPipelineMVP(
		pipeline,
		mvp);
	setTextPipelineColor(
		pipeline,
		renderHandle->color);
	return drawText(
		renderHandle->text);
}
Renderer createTextRenderer(
	Transform transform,
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sorting < RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(
		getPipelineName(pipeline),
		TEXT_PIPELINE_NAME) == 0);

	return createRenderer(
		transform,
		pipeline,
		sorting,
		useCulling,
		onRenderHandleDestroy,
		onRenderHandleDraw,
		capacity);
}
Render createTextRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Text text)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assertLinearColor(color);
	assert(text != NULL);

	assert(getTransformTransformer(
		getRendererTransform(renderer)) ==
		getTransformTransformer(transform));
	assert(getFramebufferWindow(
		getPipelineFramebuffer(
		getRendererPipeline(renderer))) ==
		getFramebufferWindow(
		getPipelineFramebuffer(
		getTextPipeline(text))));
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		TEXT_PIPELINE_NAME) == 0);

	RenderHandle* renderHandle = malloc(
		sizeof(RenderHandle));

	if (renderHandle == NULL)
		return NULL;

	renderHandle->color = color;
	renderHandle->text = text;

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

LinearColor getTextRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	return renderHandle->color;
}
void setTextRenderColor(
	Render render,
	LinearColor color)
{
	assert(render != NULL);
	assertLinearColor(color);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	renderHandle->color = color;
}

Text getTextRenderText(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	return renderHandle->text;
}
void setTextRenderText(
	Render render,
	Text text)
{
	assert(render != NULL);
	assert(text != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	renderHandle->text = text;
}
