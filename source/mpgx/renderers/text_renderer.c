#include "mpgx/renderers/text_renderer.h"
#include <string.h>

typedef struct TextRender
{
	Vec4F color;
	Text text;
} TextRender;

static void onTextRenderDestroy(void* render)
{
	free(render);
}
static void onTextRenderDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	TextRender* textRender =
		getRenderHandle(render);

	setTextPipelineMVP(
		pipeline,
		*mvp);
	setTextPipelineColor(
		pipeline,
		textRender->color);
	drawText(
		textRender->text,
		pipeline);
}
Renderer createTextRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sortingType < RENDER_SORTING_COUNT);

	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);

	return createRenderer(
		transform,
		pipeline,
		sortingType,
		onTextRenderDestroy,
		onTextRenderDraw);
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
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(renderer)),
		"Text") == 0);

	TextRender* textRender = malloc(
		sizeof(TextRender));

	if (textRender == NULL)
		return NULL;

	textRender->color = color;
	textRender->text = text;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		textRender);

	if (render == NULL)
	{
		free(textRender);
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

	TextRender* textRender =
		getRenderHandle(render);
	return textRender->color;
}
void setTextRenderColor(
	Render render,
	Vec4F color)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Text") == 0);

	TextRender* textRender =
		getRenderHandle(render);
	textRender->color = color;
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

	TextRender* textRender =
		getRenderHandle(render);
	return textRender->text;
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

	TextRender* textRender =
		getRenderHandle(render);
	textRender->text = text;
}
