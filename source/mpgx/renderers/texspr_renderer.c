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
	free((TexSprRender*)render);
}
static size_t onTexSprRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	TexSprRender* handle =
		getRenderHandle(render);
	Mesh mesh = handle->mesh;
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTexSprPipelineMVP(
		pipeline,
		mvp);
	setTexSprPipelineColor(
		pipeline,
		handle->color);
	setTexSprPipelineSize(
		pipeline,
		handle->size);
	setTexSprPipelineOffset(
		pipeline,
		handle->offset);
	drawMesh(
		mesh,
		pipeline);
	return getMeshIndexCount(mesh) / 3;
}
Renderer createTexSprRenderer(
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
		"TexSpr") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
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
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"TexSpr") == 0);

	TexSprRender* handle = malloc(
		sizeof(TexSprRender));

	if (handle == NULL)
		return NULL;

	handle->color = color;
	handle->size = size;
	handle->offset = offset;
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

Vec4F getTexSprRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexSpr") == 0);
	TexSprRender* handle =
		getRenderHandle(render);
	return handle->color;
}
void setTexSprRenderColor(
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
		"TexSpr") == 0);
	TexSprRender* handle =
		getRenderHandle(render);
	handle->color = color;
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
	TexSprRender* handle =
		getRenderHandle(render);
	return handle->size;
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
	TexSprRender* handle =
		getRenderHandle(render);
	handle->size = size;
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
	TexSprRender* handle =
		getRenderHandle(render);
	return handle->offset;
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
	TexSprRender* handle =
		getRenderHandle(render);
	handle->offset = offset;
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
	TexSprRender* handle =
		getRenderHandle(render);
	return handle->mesh;
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
	TexSprRender* handle =
		getRenderHandle(render);
	handle->mesh = mesh;
}
