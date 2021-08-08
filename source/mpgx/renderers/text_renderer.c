#include "mpgx/renderers/text_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct TextRender
{
	Vec4F color;
	Text text;
} TextRender;

static void onTextRenderDestroy(void* render)
{
	free((TextRender*)render);
}
static size_t onTextRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	TextRender* handle =
		getRenderHandle(render);
	Text text = handle->text;
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTextPipelineMVP(
		pipeline,
		mvp);
	setTextPipelineColor(
		pipeline,
		handle->color);
	drawText(
		text,
		pipeline);
	return getTextIndexCount(text) / 3;
}
Renderer createTextRenderer(
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
		"Text") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		useCulling,
		onTextRenderDestroy,
		onTextRenderDraw,
		capacity);
}
Render createTextRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Text text)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assert(text != NULL);

	assert(getTransformTransformer(
		getRendererTransform(renderer)) ==
		getTransformTransformer(transform));
	assert(getPipelineWindow(
		getRendererPipeline(renderer)) ==
		getTextWindow(text));
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"Text") == 0);

	TextRender* handle = malloc(
		sizeof(TextRender));

	if (handle == NULL)
		return NULL;

	handle->color = color;
	handle->text = text;

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

Vec4F getTextRenderColor(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Text") == 0);
	TextRender* handle =
		getRenderHandle(render);
	return handle->color;
}
void setTextRenderColor(
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
		"Text") == 0);
	TextRender* handle =
		getRenderHandle(render);
	handle->color = color;
}

Text getTextRenderText(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Text") == 0);
	TextRender* handle =
		getRenderHandle(render);
	return handle->text;
}
void setTextRenderText(
	Render render,
	Text text)
{
	assert(render != NULL);
	assert(text != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Text") == 0);
	TextRender* handle =
		getRenderHandle(render);
	handle->text = text;
}
