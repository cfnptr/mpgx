#include "mpgx/renderers/diffuse_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct DiffuseRender
{
	Mesh mesh;
} DiffuseRender;

static void onDiffuseRenderDestroy(void* render)
{
	free((DiffuseRender*)render);
}
static size_t onDiffuseRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	DiffuseRender* handle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	Mat4F normal = transposeMat4F(
		invMat4F(*model));
	setDiffusePipelineMVP(
		pipeline,
		mvp);
	setDiffusePipelineNormal(
		pipeline,
		normal);
	return drawMesh(
		handle->mesh,
		pipeline);
}
Renderer createDiffuseRenderer(
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
		"Diffuse") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onDiffuseRenderDestroy,
		onDiffuseRenderDraw,
		capacity);
}
Render createDiffuseRender(
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
		"Diffuse") == 0);

	DiffuseRender* handle = malloc(
		sizeof(DiffuseRender));

	if (handle == NULL)
		return NULL;

	handle->mesh = mesh;

	Render render = createRender(
		renderer,
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

Mesh getDiffuseRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Diffuse") == 0);
	DiffuseRender* handle =
		getRenderHandle(render);
	return handle->mesh;
}
void setDiffuseRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Diffuse") == 0);
	DiffuseRender* handle =
		getRenderHandle(render);
	handle->mesh = mesh;
}
