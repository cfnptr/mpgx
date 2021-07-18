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
	free(render);
}
static void onSpriteRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	SpriteRender* spriteRender =
		getRenderHandle(render);

	setSpritePipelineMVP(
		pipeline,
		*mvp);
	setSpritePipelineColor(
		pipeline,
		spriteRender->color);
	drawMesh(
		spriteRender->mesh,
		pipeline);
}
Renderer createSpriteRenderer(
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
		"Sprite") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
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
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"Sprite") == 0);

	SpriteRender* spriteRender = malloc(
		sizeof(SpriteRender));

	if (spriteRender == NULL)
		return NULL;

	spriteRender->color = color;
	spriteRender->mesh = mesh;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		spriteRender);

	if (render == NULL)
	{
		free(spriteRender);
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

	SpriteRender* spriteRender =
		getRenderHandle(render);
	return spriteRender->color;
}
void setSpriteRenderColor(
	Render render,
	Vec4F color)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Sprite") == 0);

	SpriteRender* spriteRender =
		getRenderHandle(render);
	spriteRender->color = color;
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

	SpriteRender* spriteRender =
		getRenderHandle(render);
	return spriteRender->mesh;
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

	SpriteRender* spriteRender =
		getRenderHandle(render);
	spriteRender->mesh = mesh;
}
