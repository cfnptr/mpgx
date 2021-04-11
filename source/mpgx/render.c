#include "mpgx/render.h"
#include "mpgx/pipeline.h"

typedef struct ColorRender
{
	Vector4F color;
	Mesh* mesh;
} ColorRender;
typedef struct SpriteRender
{
	Vector4F color;
	Mesh* mesh;
} SpriteRender;
typedef struct DiffuseRender
{
	Mesh* mesh;
} DiffuseRender;
typedef struct TextRender
{
	Vector4F color;
	Text* text;
} TextRender;

static void destroyColorRender(void* render)
{
	free(render);
}
static void renderColorCommand(
	Render* render,
	Pipeline* pipeline,
	const Matrix4F* model,
	const Matrix4F* view,
	const Matrix4F* proj,
	const Matrix4F* viewProj,
	const Matrix4F* mvp)
{
	ColorRender* colorRender =
		getRenderHandle(render);

	setColorPipelineMVP(
		pipeline,
		*mvp);
	setColorPipelineColor(
		pipeline,
		colorRender->color);
	drawMeshCommand(
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

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		destroyColorRender,
		renderColorCommand);
}
Render* createColorRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
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

	ColorRender* colorRender = malloc(
		sizeof(ColorRender));

	if (colorRender == NULL)
		return NULL;

	colorRender->color = color;
	colorRender->mesh = mesh;

	Render* render = createRender(
		renderer,
		transform,
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

	ColorRender* colorRender =
		getRenderHandle(render);
	colorRender->mesh = mesh;
}

Vector4F getColorRenderColor(
	const Render* render)
{
	assert(render != NULL);
	ColorRender* colorRender =
		getRenderHandle(render);
	return colorRender->color;
}
void setColorRenderColor(
	Render* render,
	Vector4F color)
{
	assert(render != NULL);
	ColorRender* colorRender =
		getRenderHandle(render);
	colorRender->color = color;
}

static void destroySpriteRender(
	void* render)
{
	free(render);
}
static void renderSpriteCommand(
	Render* render,
	Pipeline* pipeline,
	const Matrix4F* model,
	const Matrix4F* view,
	const Matrix4F* proj,
	const Matrix4F* viewProj,
	const Matrix4F* mvp)
{
	SpriteRender* spriteRender =
		getRenderHandle(render);

	setSpritePipelineMVP(
		pipeline,
		*mvp);
	setSpritePipelineColor(
		pipeline,
		spriteRender->color);
	drawMeshCommand(
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

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		destroySpriteRender,
		renderSpriteCommand);
}
Render* createSpriteRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
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

	SpriteRender* spriteRender = malloc(
		sizeof(SpriteRender));

	if (spriteRender == NULL)
		return NULL;

	spriteRender->color = color;
	spriteRender->mesh = mesh;

	Render* render = createRender(
		renderer,
		transform,
		spriteRender);

	if (render == NULL)
	{
		free(spriteRender);
		return NULL;
	}

	return render;
}

Vector4F getSpriteRenderColor(
	const Render* render)
{
	assert(render != NULL);
	SpriteRender* spriteRender =
		getRenderHandle(render);
	return spriteRender->color;
}
void setSpriteRenderColor(
	Render* render,
	Vector4F color)
{
	assert(render != NULL);
	SpriteRender* spriteRender =
		getRenderHandle(render);
	spriteRender->color = color;
}

Mesh* getSpriteRenderMesh(
	const Render* render)
{
	assert(render != NULL);
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

	SpriteRender* spriteRender =
		getRenderHandle(render);
	spriteRender->mesh = mesh;
}

static void destroyDiffuseRender(
	void* render)
{
	free(render);
}
static void renderDiffuseCommand(
	Render* render,
	Pipeline* pipeline,
	const Matrix4F* model,
	const Matrix4F* view,
	const Matrix4F* proj,
	const Matrix4F* viewProj,
	const Matrix4F* mvp)
{
	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	Matrix4F normal = invMat4F(
		transposeMat4F(*model));

	setDiffusePipelineMVP(
		pipeline,
		*mvp);
	setDiffusePipelineNormal(
		pipeline,
		normal);
	drawMeshCommand(
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

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		destroyDiffuseRender,
		renderDiffuseCommand);
}
Render* createDiffuseRender(
	Renderer* renderer,
	Transform* transform,
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

	DiffuseRender* diffuseRender = malloc(
		sizeof(DiffuseRender));

	if (diffuseRender == NULL)
		return NULL;

	diffuseRender->mesh = mesh;

	Render* render = createRender(
		renderer,
		transform,
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

	DiffuseRender* diffuseRender =
		getRenderHandle(render);
	diffuseRender->mesh = mesh;
}

static void destroyTextRender(
	void* render)
{
	free(render);
}
static void renderTextCommand(
	Render* render,
	Pipeline* pipeline,
	const Matrix4F* model,
	const Matrix4F* view,
	const Matrix4F* proj,
	const Matrix4F* viewProj,
	const Matrix4F* mvp)
{
	TextRender* textRender =
		getRenderHandle(render);

	setTextPipelineMVP(
		pipeline,
		*mvp);
	setTextPipelineColor(
		pipeline,
		textRender->color);
	drawTextCommand(
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

	return createRenderer(
		transform,
		pipeline,
		ascendingSorting,
		destroyTextRender,
		renderTextCommand);
}
Render* createTextRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
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

	TextRender* textRender = malloc(
		sizeof(TextRender));

	if (textRender == NULL)
		return NULL;

	textRender->color = color;
	textRender->text = text;

	Render* render = createRender(
		renderer,
		transform,
		textRender);

	if (render == NULL)
	{
		free(textRender);
		return NULL;
	}

	return render;
}

Vector4F getTextRenderColor(
	const Render* render)
{
	assert(render != NULL);
	TextRender* textRender =
		getRenderHandle(render);
	return textRender->color;
}
void setTextRenderColor(
	Render* render,
	Vector4F color)
{
	assert(render != NULL);
	TextRender* textRender =
		getRenderHandle(render);
	textRender->color = color;
}

Text* getTextRenderText(
	const Render* render)
{
	assert(render != NULL);
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

	TextRender* textRender =
		getRenderHandle(render);
	textRender->text = text;
}
