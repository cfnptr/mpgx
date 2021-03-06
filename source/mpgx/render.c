#include "mpgx/render.h"
#include "mpgx/pipeline.h"

struct ColorRender
{
	struct Mesh* mesh;
};
struct SpriteRender
{
	struct Vector4F color;
	struct Mesh* mesh;
};
struct TextRender
{
	struct Vector4F color;
	struct Text* text;
};

void destroyColorRender(
	void* render)
{
	free(render);
}
void renderColorCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp)
{
	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);

	setColorPipelineMVP(
		pipeline,
		*mvp);
	drawMeshCommand(
		colorRender->mesh,
		pipeline);
}
struct Render* createColorRender(
	struct Renderer* renderer,
	bool draw,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Render* parent,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);

	assert(getPipelineWindow(
		getRendererPipeline(renderer)) == getMeshWindow(mesh));

	struct ColorRender* coloRender = malloc(
		sizeof(struct ColorRender));

	if (coloRender == NULL)
		return NULL;

	coloRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		draw,
		position,
		scale,
		rotation,
		parent,
		destroyColorRender,
		renderColorCommand,
		coloRender);

	if (render == NULL)
	{
		free(coloRender);
		return NULL;
	}

	return render;
}

struct Mesh* getColorRenderMesh(
	const struct Render* render)
{
	assert(render != NULL);

	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);
	return colorRender->mesh;
}
void setColorRenderMesh(
	struct Render* render,
	struct Mesh* mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);
	colorRender->mesh = mesh;
}

void destroySpriteRender(
	void* render)
{
	free(render);
}
void renderSpriteCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp)
{
	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);

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
struct Render* createSpriteRender(
	struct Renderer* renderer,
	bool draw,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Render* parent,
	struct Vector4F color,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);

	assert(getPipelineWindow(
		getRendererPipeline(renderer)) == getMeshWindow(mesh));

	struct SpriteRender* spriteRender = malloc(
		sizeof(struct SpriteRender));

	if (spriteRender == NULL)
		return NULL;

	spriteRender->color = color;
	spriteRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		draw,
		position,
		scale,
		rotation,
		parent,
		destroySpriteRender,
		renderSpriteCommand,
		spriteRender);

	if (render == NULL)
	{
		free(spriteRender);
		return NULL;
	}

	return render;
}

struct Vector4F getSpriteRenderColor(
	const struct Render* render)
{
	assert(render != NULL);

	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);
	return spriteRender->color;
}
void setSpriteRenderColor(
	struct Render* render,
	struct Vector4F color)
{
	assert(render != NULL);

	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);
	spriteRender->color = color;
}

struct Mesh* getSpriteRenderMesh(
	const struct Render* render)
{
	assert(render != NULL);

	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);
	return spriteRender->mesh;
}
void setSpriteRenderMesh(
	struct Render* render,
	struct Mesh* mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);
	spriteRender->mesh = mesh;
}

void destroyTextRender(
	void* render)
{
	free(render);
}
void renderTextCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp)
{
	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);

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
struct Render* createTextRender(
	struct Renderer* renderer,
	bool draw,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Render* parent,
	struct Vector4F color,
	struct Text* text)
{
	assert(renderer != NULL);
	assert(text != NULL);

	assert(getPipelineWindow(
		getRendererPipeline(renderer)) == getTextWindow(text));

	struct TextRender* textRender = malloc(
		sizeof(struct TextRender));

	if (textRender == NULL)
		return NULL;

	textRender->color = color;
	textRender->text = text;

	struct Render* render = createRender(
		renderer,
		draw,
		position,
		scale,
		rotation,
		parent,
		destroyTextRender,
		renderTextCommand,
		textRender);

	if (render == NULL)
	{
		free(textRender);
		return NULL;
	}

	return render;
}

struct Vector4F getTextRenderColor(
	const struct Render* render)
{
	assert(render != NULL);

	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);
	return textRender->color;
}
void setTextRenderColor(
	struct Render* render,
	struct Vector4F color)
{
	assert(render != NULL);

	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);
	textRender->color = color;
}

struct Text* getTextRenderText(
	const struct Render* render)
{
	assert(render != NULL);

	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);
	return textRender->text;
}
void setTextRenderText(
	struct Render* render,
	struct Text* text)
{
	assert(render != NULL);
	assert(text != NULL);

	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);
	textRender->text = text;
}
