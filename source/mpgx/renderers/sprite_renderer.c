#include "mpgx/renderers/sprite_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct SpriteRender
{
	Vec4F color;
	Mesh mesh;
} SpriteRender;

static void onSpriteRenderDestroy(void* render)
{
	free((SpriteRender*)render);
}
static size_t onSpriteRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	SpriteRender* handle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setSpritePipelineMvp(
		pipeline,
		mvp);
	setSpritePipelineColor(
		pipeline,
		handle->color);
	return drawMesh(
		handle->mesh,
		pipeline);
}
Renderer createSpriteRenderer(
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
		"Sprite") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onSpriteRenderDestroy,
		onSpriteRenderDraw,
		capacity);
}
Render createSpriteRender(
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
		"Sprite") == 0);

	SpriteRender* handle = malloc(
		sizeof(SpriteRender));

	if (handle == NULL)
		return NULL;

	handle->color = color;
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

Vec4F getSpriteRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Sprite") == 0);
	SpriteRender* handle =
		getRenderHandle(render);
	return handle->color;
}
void setSpriteRenderColor(
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
		"Sprite") == 0);
	SpriteRender* handle =
		getRenderHandle(render);
	handle->color = color;
}

Mesh getSpriteRenderMesh(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Sprite") == 0);
	SpriteRender* handle =
		getRenderHandle(render);
	return handle->mesh;
}
void setSpriteRenderMesh(
	Render render,
	Mesh mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Sprite") == 0);
	SpriteRender* handle =
		getRenderHandle(render);
	handle->mesh = mesh;
}
