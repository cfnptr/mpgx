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
	bool _render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);
	assert(getRendererWindow(renderer) == getMeshWindow(mesh));

	struct ColorRender* coloRender = malloc(
		sizeof(struct ColorRender));

	if (coloRender == NULL)
		return NULL;

	coloRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		_render,
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
	bool _render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Vector4F color,
	struct Transform* parent,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);
	assert(getRendererWindow(renderer) == getMeshWindow(mesh));

	struct SpriteRender* spriteRender = malloc(
		sizeof(struct SpriteRender));

	if (spriteRender == NULL)
		return NULL;

	spriteRender->color = color;
	spriteRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		_render,
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
	drawTextCommand(
		textRender->text,
		pipeline);
}
struct Render* createTextRender(
	struct Renderer* renderer,
	bool _render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	struct Text* text)
{
	assert(renderer != NULL);
	assert(text != NULL);
	assert(getRendererWindow(renderer) == getTextWindow(text));

	struct TextRender* textRender = malloc(
		sizeof(struct TextRender));

	if (textRender == NULL)
		return NULL;

	textRender->text = text;

	struct Render* render = createRender(
		renderer,
		_render,
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
