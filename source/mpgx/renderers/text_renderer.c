#include "mpgx/renderers/text_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct RenderHandle
{
	Vec4F color;
	Text text;
} RenderHandle;

static void onRenderHandleDestroy(void* handle)
{
	free((RenderHandle*)handle);
}
static size_t onRenderHandleDraw(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	RenderHandle* renderHandle =
		getRenderHandle(render);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setTextPipelineMVP(
		pipeline,
		mvp);
	setTextPipelineColor(
		pipeline,
		renderHandle->color);
	return drawText(
		renderHandle->text,
		pipeline);
}
Renderer createTextRenderer(
	Transform transform,
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity)
{
	assert(transform != NULL);
	assert(pipeline != NULL);
	assert(sorting < RENDER_SORTING_COUNT);
	assert(capacity != 0);

	assert(strcmp(
		getPipelineName(pipeline),
		TEXT_PIPELINE_NAME) == 0);

	return createRenderer(
		transform,
		pipeline,
		sorting,
		useCulling,
		onRenderHandleDestroy,
		onRenderHandleDraw,
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
		TEXT_PIPELINE_NAME) == 0);

	RenderHandle* renderHandle = malloc(
		sizeof(RenderHandle));

	if (renderHandle == NULL)
		return NULL;

	renderHandle->color = color;
	renderHandle->text = text;

	Render render = createRender(
		renderer,
		transform,
		bounding,
		renderHandle);

	if (render == NULL)
	{
		free(renderHandle);
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
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	return renderHandle->color;
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
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	renderHandle->color = color;
}

Text getTextRenderText(
	Render render)
{
	assert(render != NULL);
	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	return renderHandle->text;
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
		TEXT_PIPELINE_NAME) == 0);
	RenderHandle* renderHandle =
		getRenderHandle(render);
	renderHandle->text = text;
}
