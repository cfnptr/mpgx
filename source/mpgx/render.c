#include "mpgx/render.h"
#include "mpgx/pipeline.h"

#include <string.h>

typedef struct ColorRender
{
	Vec4F color;
	Mesh* mesh;
} ColorRender;
typedef struct SpriteRender
{
	Vec4F color;
	Mesh* mesh;
} SpriteRender;
typedef struct DiffuseRender
{
	Mesh* mesh;
} DiffuseRender;
typedef struct TextRender
{
	Vec4F color;
	Text* text;
} TextRender;

static void onColorRenderDestroy(void* render)
{
	free(render);
}
static void onColorRenderDraw(
	Render* render,
	Pipeline* pipeline,
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
Renderer* createColorRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting)
{
	assert(transform != NULL);
	assert(pipeline != NULL);

	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		onColorRenderDestroy,
		onColorRenderDraw);
}
Render* createColorRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Mesh* mesh)
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

	Render* render = createRender(
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

Mesh* getColorRenderMesh(
	const Render* render)
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
	Render* render,
	Mesh* mesh)
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

Vec4F getColorRenderColor(
	const Render* render)
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
	Render* render,
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

static void onSpriteRenderDestroy(
	void* render)
{
	free(render);
}
static void onSpriteRenderDraw(
	Render* render,
	Pipeline* pipeline,
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
Renderer* createSpriteRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting)
{
	assert(transform != NULL);
	assert(pipeline != NULL);

	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		onSpriteRenderDestroy,
		onSpriteRenderDraw);
}
Render* createSpriteRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Mesh* mesh)
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

	Render* render = createRender(
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
	const Render* render)
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
	Render* render,
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

Mesh* getSpriteRenderMesh(
	const Render* render)
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
	Render* render,
	Mesh* mesh)
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

static void onDiffuseRenderDestroy(
	void* render)
{
	free(render);
}
static void onDiffuseRenderDraw(
	Render* render,
	Pipeline* pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp)
{
	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	Mat4F normal = invMat4F(
		transposeMat4F(*model));

	setDiffusePipelineMVP(
		pipeline,
		*mvp);
	setDiffusePipelineNormal(
		pipeline,
		normal);
	drawMesh(
		diffuseRender->mesh,
		pipeline);
}
Renderer* createDiffuseRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting)
{
	assert(transform != NULL);
	assert(pipeline != NULL);

	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		onDiffuseRenderDestroy,
		onDiffuseRenderDraw);
}
Render* createDiffuseRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Mesh* mesh)
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
		"Diffuse") == 0);

	DiffuseRender* diffuseRender = malloc(
		sizeof(DiffuseRender));

	if (diffuseRender == NULL)
		return NULL;

	diffuseRender->mesh = mesh;

	Render* render = createRender(
		renderer,
		transform,
		bounding,
		diffuseRender);

	if (render == NULL)
	{
		free(diffuseRender);
		return NULL;
	}

	return render;
}

Mesh* getDiffuseRenderMesh(
	const Render* render)
{
	assert(render != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Diffuse") == 0);

	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	return diffuseRender->mesh;
}
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	assert(strcmp(
		getPipelineName(
		getRendererPipeline(
		getRenderRenderer(render))),
		"Diffuse") == 0);

	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	diffuseRender->mesh = mesh;
}

static void onTextRenderDestroy(
	void* render)
{
	free(render);
}
static void onTextRenderDraw(
	Render* render,
	Pipeline* pipeline,
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
Renderer* createTextRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting)
{
	assert(transform != NULL);
	assert(pipeline != NULL);

	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		onTextRenderDestroy,
		onTextRenderDraw);
}
Render* createTextRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Text* text)
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

	Render* render = createRender(
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
	const Render* render)
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
	Render* render,
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

Text* getTextRenderText(
	const Render* render)
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
	Render* render,
	Text* text)
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
