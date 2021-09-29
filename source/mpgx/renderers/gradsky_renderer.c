#include "mpgx/renderers/gradsky_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct RenderHandle
{
	Mesh mesh;
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
	setGradSkyPipelineMvp(
		pipeline,
		mvp);
	return drawMesh(
		renderHandle->mesh,
		pipeline);
}
Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onRenderHandleDestroy,
		onRenderHandleDraw,
		capacity);
}
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Mesh mesh)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assert(mesh != NULL);

	assert(getTransformTransformer(
		getRendererTransform(renderer)) ==
		getTransformTransformer(transform));
	assert(getPipelineWindow(
		getRendererPipeline(renderer)) ==
		getMeshWindow(mesh));
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		GRAD_SKY_PIPELINE_NAME) == 0);

	RenderHandle* renderHandle = malloc(
		sizeof(RenderHandle));

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

Mesh getGradSkyRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		GRAD_SKY_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	return renderHandle->mesh;
}
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		GRAD_SKY_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	renderHandle->mesh = mesh;
}
