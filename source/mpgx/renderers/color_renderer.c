#include "mpgx/renderers/color_renderer.h"
#include <string.h>

typedef struct ColorRender
{
	Vec4F color;
	Mesh mesh;
} ColorRender;

static void onColorRenderDestroy(void* render)
{
	free(render);
}
static void onColorRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	ColorRender* colorRender =
		getRenderHandle(render);

	setColorPipelineMVP(
		pipeline,
		*mvp);
	setColorPipelineColor(
		pipeline,
		colorRender->color);
	drawMesh(
		colorRender->mesh,
		pipeline);
}
Renderer createColorRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);

	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		onColorRenderDestroy,
		onColorRenderDraw);
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
