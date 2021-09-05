#include "mpgx/renderers/gradsky_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct GradSkyRender
{
	float sunHeight;
	Mesh mesh;
} GradSkyRender;

static void onGradSkyRenderDestroy(void* render)
{
	free((GradSkyRender*)render);
}
static size_t onGradSkyRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	GradSkyRender* handle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setGradSkyPipelineMvp(
		pipeline,
		mvp);
	setGradSkyPipelineSunHeight(
		pipeline,
		handle->sunHeight);
	return drawMesh(
		handle->mesh,
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
		"GradSky") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onGradSkyRenderDestroy,
		onGradSkyRenderDraw,
		capacity);
}
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	float sunHeight,
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
		"GradSky") == 0);

	GradSkyRender* handle = malloc(
		sizeof(GradSkyRender));

	if (handle == NULL)
		return NULL;

	handle->sunHeight = sunHeight;
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

float getGradSkyRenderSunHeight(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* handle =
		getRenderHandle(render);
	return handle->sunHeight;
}
void setGradSkyRenderSunHeight(
	Render render,
	float sunHeight)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* handle =
		getRenderHandle(render);
	handle->sunHeight = sunHeight;
}

Mesh getGradSkyRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* handle =
		getRenderHandle(render);
	return handle->mesh;
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
		"GradSky") == 0);
	GradSkyRender* handle =
		getRenderHandle(render);
	handle->mesh = mesh;
}
