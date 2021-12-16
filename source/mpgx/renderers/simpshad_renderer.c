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

#include "mpgx/renderers/simpshad_renderer.h"

#include <string.h>
#include <assert.h>

struct RenderHandle_T
{
	Mesh mesh;
};

typedef struct RenderHandle_T RenderHandle_T;
typedef RenderHandle_T* RenderHandle;

static void onRenderHandleDestroy(void* handle)
{
	free((RenderHandle)handle);
}
static size_t onRenderHandleDraw(
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
	setSimpShadPipelineMvp(
		pipeline,
		mvp);
	return drawMesh(
		renderHandle->mesh,
		pipeline);
}
Renderer createSimpShadRenderer(
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
		SIMPSHAD_PIPELINE_NAME) == 0);

	return createRenderer(
		pipeline,
		sorting,
		useCulling,
		onRenderHandleDestroy,
		onRenderHandleDraw,
		capacity);
}
Render createSimpShadRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
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
		SIMPSHAD_PIPELINE_NAME) == 0);

	RenderHandle renderHandle = malloc(
		sizeof(RenderHandle_T));

	if (renderHandle == NULL)
		return NULL;

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

Mesh getSimpShadRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		SIMPSHAD_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	return renderHandle->mesh;
}
void setSimpShadRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		SIMPSHAD_PIPELINE_NAME) == 0);
	RenderHandle renderHandle =
		getRenderHandle(render);
	renderHandle->mesh = mesh;
}
