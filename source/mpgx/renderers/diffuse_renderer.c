#include "mpgx/renderers/diffuse_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct DiffuseRender
{
	Mesh mesh;
} DiffuseRender;

static void onDiffuseRenderDestroy(void* render)
{
	free(render);
}
static void onDiffuseRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	Mat4F normal = invMat4F(
		transposeMat4F(*model));

	setDiffusePipelineMVP(
		pipeline,
		*mvp);
	setDiffusePipelineNormal(
		pipeline,
		normal);
	drawMesh(
		diffuseRender->mesh,
		pipeline);
}
Renderer createDiffuseRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
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

	DiffuseRender* diffuseRender = malloc(
		sizeof(DiffuseRender));

	if (diffuseRender == NULL)
		return NULL;

	diffuseRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		diffuseRender);

	if (render == NULL)
	{
		free(diffuseRender);
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

	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	return diffuseRender->mesh;
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

	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	diffuseRender->mesh = mesh;
}
