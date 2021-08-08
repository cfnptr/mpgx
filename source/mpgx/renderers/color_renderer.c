#include "mpgx/renderers/color_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct ColorRender
{
	Vec4F color;
	Mesh mesh;
} ColorRender;

static void onColorRenderDestroy(void* render)
{
	free((ColorRender*)render);
}
static size_t onColorRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	ColorRender* handle =
		getRenderHandle(render);
	Mesh mesh = handle->mesh;
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setColorPipelineMVP(
		pipeline,
		mvp);
	setColorPipelineColor(
		pipeline,
		handle->color);
	drawMesh(
		mesh,
		pipeline);
	return getMeshIndexCount(mesh) / 3;
}
Renderer createColorRenderer(
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
		"Color") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onColorRenderDestroy,
		onColorRenderDraw,
		capacity);
}
Render createColorRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
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
		"Color") == 0);

	ColorRender* colorRender = malloc(
		sizeof(ColorRender));

	if (colorRender == NULL)
		return NULL;

	colorRender->color = color;
	colorRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		colorRender);

	if (render == NULL)
	{
		free(colorRender);
		return NULL;
	}

	return render;
}

Vec4F getColorRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Color") == 0);
	ColorRender* colorRender =
		getRenderHandle(render);
	return colorRender->color;
}
void setColorRenderColor(
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
		"Color") == 0);
	ColorRender* colorRender =
		getRenderHandle(render);
	colorRender->color = color;
}

Mesh getColorRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Color") == 0);
	ColorRender* colorRender =
		getRenderHandle(render);
	return colorRender->mesh;
}
void setColorRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Color") == 0);
	ColorRender* colorRender =
		getRenderHandle(render);
	colorRender->mesh = mesh;
}
