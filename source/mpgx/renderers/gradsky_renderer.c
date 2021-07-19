#include "mpgx/renderers/gradsky_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct GradSkyRender
{
	Vec4F color;
	float time;
	Mesh mesh;
} GradSkyRender;

static void onGradSkyRenderDestroy(void* render)
{
	free(render);
}
static void onGradSkyRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);

	setGradSkyPipelineMVP(
		pipeline,
		*mvp);
	setGradSkyPipelineColor(
		pipeline,
		gradSkyRender->color);
	setGradSkyPipelineTime(
		pipeline,
		gradSkyRender->time);
	drawMesh(
		gradSkyRender->mesh,
		pipeline);
}
Renderer createGradSkyRenderer(
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
		"GradSky") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		onGradSkyRenderDestroy,
		onGradSkyRenderDraw,
		capacity);
}
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	float time,
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
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"GradSky") == 0);

	GradSkyRender* gradSkyRender = malloc(
		sizeof(GradSkyRender));

	if (gradSkyRender == NULL)
		return NULL;

	gradSkyRender->color = color;
	gradSkyRender->time = time;
	gradSkyRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		gradSkyRender);

	if (render == NULL)
	{
		free(gradSkyRender);
		return NULL;
	}

	return render;
}

Vec4F getGradSkyRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	return gradSkyRender->color;
}
void setGradSkyRenderColor(
	Render render,
	Vec4F color)
{
	assert(render != NULL);
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	gradSkyRender->color = color;
}

float getGradSkyRenderTime(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	return gradSkyRender->time;
}
void setGradSkyRenderTime(
	Render render,
	float time)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"GradSky") == 0);
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	gradSkyRender->time = time;
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
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	return gradSkyRender->mesh;
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
	GradSkyRender* gradSkyRender =
		getRenderHandle(render);
	gradSkyRender->mesh = mesh;
}
