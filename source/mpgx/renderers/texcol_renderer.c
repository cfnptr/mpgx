#include "mpgx/renderers/texcol_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct TexColRender
{
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	Mesh mesh;
} TexColRender;

static void onTexColRenderDestroy(void* render)
{
	free((TexColRender*)render);
}
static size_t onTexColRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	TexColRender* handle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTexColPipelineMVP(
		pipeline,
		mvp);
	setTexColPipelineColor(
		pipeline,
		handle->color);
	setTexColPipelineSize(
		pipeline,
		handle->size);
	setTexColPipelineOffset(
		pipeline,
		handle->offset);
	return drawMesh(
		handle->mesh,
		pipeline);
}
Renderer createTexColRenderer(
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
		"TexCol") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onTexColRenderDestroy,
		onTexColRenderDraw,
		capacity);
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
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"TexCol") == 0);

	TexColRender* handle = malloc(
		sizeof(TexColRender));

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

Vec4F getTexColRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"TexCol") == 0);
	TexColRender* handle =
		getRenderHandle(render);
	return handle->color;
}
void setTexColRenderColor(
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
		"TexCol") == 0);
	TexColRender* handle =
		getRenderHandle(render);
	handle->color = color;
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
	TexColRender* handle =
		getRenderHandle(render);
	return handle->size;
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
	TexColRender* handle =
		getRenderHandle(render);
	handle->size = size;
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
	TexColRender* handle =
		getRenderHandle(render);
	return handle->offset;
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
	TexColRender* handle =
		getRenderHandle(render);
	handle->offset = offset;
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
	TexColRender* handle =
		getRenderHandle(render);
	return handle->mesh;
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
	TexColRender* handle =
		getRenderHandle(render);
	handle->mesh = mesh;
}
