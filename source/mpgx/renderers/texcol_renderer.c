#include "mpgx/renderers/texcol_renderer.h"
#include <string.h>

typedef struct TexColRender
{
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	Mesh mesh;
} TexColRender;

static void onTexColRenderDestroy(void* render)
{
	free(render);
}
static void onTexColRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	TexColRender* texColRender =
		getRenderHandle(render);

	setTexColPipelineMVP(
		pipeline,
		*mvp);
	setTexColPipelineColor(
		pipeline,
		texColRender->color);
	setTexColPipelineSize(
		pipeline,
		texColRender->size);
	setTexColPipelineOffset(
		pipeline,
		texColRender->offset);
	drawMesh(
		texColRender->mesh,
		pipeline);
}
Renderer createTexColRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);

	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		onTexColRenderDestroy,
		onTexColRenderDraw);
}
Render createTexColRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
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
		"TexCol") == 0);

	TexColRender* texColRender = malloc(
		sizeof(TexColRender));

	if (texColRender == NULL)
		return NULL;

	texColRender->color = color;
	texColRender->size = size;
	texColRender->offset = offset;
	texColRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		texColRender);

	if (render == NULL)
	{
		free(texColRender);
		return NULL;
	}

	return render;
}

Vec4F getTexColRenderColor(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	return texColRender->color;
}
void setTexColRenderColor(
	Render render,
	Vec4F color)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	texColRender->color = color;
}

Vec2F getTexColRenderSize(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	return texColRender->size;
}
void setTexColRenderSize(
	Render render,
	Vec2F size)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	texColRender->size = size;
}

Vec2F getTexColRenderOffset(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	return texColRender->offset;
}
void setTexColRenderOffset(
	Render render,
	Vec2F offset)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	texColRender->offset = offset;
}

Mesh getTexColRenderMesh(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	return texColRender->mesh;
}
void setTexColRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);

	TexColRender* texColRender =
		getRenderHandle(render);
	texColRender->mesh = mesh;
}
