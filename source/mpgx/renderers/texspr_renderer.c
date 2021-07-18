#include "mpgx/renderers/texspr_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct TexSprRender
{
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	Mesh mesh;
} TexSprRender;

static void onTexSprRenderDestroy(void* render)
{
	free(render);
}
static void onTexSprRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	TexSprRender* texSprRender =
		getRenderHandle(render);

	setTexSprPipelineMVP(
		pipeline,
		*mvp);
	setTexSprPipelineColor(
		pipeline,
		texSprRender->color);
	setTexSprPipelineSize(
		pipeline,
		texSprRender->size);
	setTexSprPipelineOffset(
		pipeline,
		texSprRender->offset);
	drawMesh(
		texSprRender->mesh,
		pipeline);
}
Renderer createTexSprRenderer(
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
		"TexSpr") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		onTexSprRenderDestroy,
		onTexSprRenderDraw,
		capacity);
}
Render createTexSprRender(
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
		"TexSpr") == 0);

	TexSprRender* texSprRender = malloc(
		sizeof(TexSprRender));

	if (texSprRender == NULL)
		return NULL;

	texSprRender->color = color;
	texSprRender->size = size;
	texSprRender->offset = offset;
	texSprRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		texSprRender);

	if (render == NULL)
	{
		free(texSprRender);
		return NULL;
	}

	return render;
}

Vec4F getTexSprRenderColor(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	return texSprRender->color;
}
void setTexSprRenderColor(
	Render render,
	Vec4F color)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	texSprRender->color = color;
}

Vec2F getTexSprRenderSize(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	return texSprRender->size;
}
void setTexSprRenderSize(
	Render render,
	Vec2F size)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	texSprRender->size = size;
}

Vec2F getTexSprRenderOffset(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	return texSprRender->offset;
}
void setTexSprRenderOffset(
	Render render,
	Vec2F offset)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	texSprRender->offset = offset;
}

Mesh getTexSprRenderMesh(
	Render render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	return texSprRender->mesh;
}
void setTexSprRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);

	TexSprRender* texSprRender =
		getRenderHandle(render);
	texSprRender->mesh = mesh;
}
