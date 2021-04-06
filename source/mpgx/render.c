#include "mpgx/render.h"
#include "mpgx/pipeline.h"

struct ColorRender
{
	struct Vec4F color;
	struct Mesh* mesh;
};
struct SpriteRender
{
	struct Vec4F color;
	struct Mesh* mesh;
};
struct DiffuseRender
{
	struct Mesh* mesh;
};
struct TextRender
{
	struct Vec4F color;
	struct Text* text;
};

static void destroyColorRender(
	void* render)
{
	free(render);
}
static void renderColorCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Mat4F* model,
	const struct Mat4F* view,
	const struct Mat4F* proj,
	const struct Mat4F* viewProj,
	const struct Mat4F* mvp)
{
	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);

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
struct Render* createColorRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Render* parent,
	struct Vec4F color,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);

	assert(getPipelineWindow(
		getRendererPipeline(renderer)) == getMeshWindow(mesh));

	struct ColorRender* colorRender = malloc(
		sizeof(struct ColorRender));

	if (colorRender == NULL)
		return NULL;

	colorRender->color = color;
	colorRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		draw,
		position,
		scale,
		rotation,
		rotationType,
		parent,
		destroyColorRender,
		renderColorCommand,
		colorRender);

	if (render == NULL)
	{
		free(colorRender);
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

struct Vec4F getColorRenderColor(
	const struct Render* render)
{
	assert(render != NULL);
	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);
	return colorRender->color;
}
void setColorRenderColor(
	struct Render* render,
	struct Vec4F color)
{
	assert(render != NULL);
	struct ColorRender* colorRender =
		(struct ColorRender*)getRenderHandle(render);
	colorRender->color = color;
}

static void destroySpriteRender(
	void* render)
{
	free(render);
}
static void renderSpriteCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Mat4F* model,
	const struct Mat4F* view,
	const struct Mat4F* proj,
	const struct Mat4F* viewProj,
	const struct Mat4F* mvp)
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
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Render* parent,
	struct Vec4F color,
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
		rotationType,
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

struct Vec4F getSpriteRenderColor(
	const struct Render* render)
{
	assert(render != NULL);
	struct SpriteRender* spriteRender =
		(struct SpriteRender*)getRenderHandle(render);
	return spriteRender->color;
}
void setSpriteRenderColor(
	struct Render* render,
	struct Vec4F color)
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

static void destroyDiffuseRender(
	void* render)
{
	free(render);
}
static void renderDiffuseCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Mat4F* model,
	const struct Mat4F* view,
	const struct Mat4F* proj,
	const struct Mat4F* viewProj,
	const struct Mat4F* mvp)
{
	struct DiffuseRender* diffuseRender =
		(struct DiffuseRender*)getRenderHandle(render);
	struct Mat4F normal = invMat4F(
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
struct Render* createDiffuseRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Render* parent,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(mesh != NULL);

	assert(getPipelineWindow(
		getRendererPipeline(renderer)) == getMeshWindow(mesh));

	struct DiffuseRender* diffuseRender = malloc(
		sizeof(struct DiffuseRender));

	if (diffuseRender == NULL)
		return NULL;

	diffuseRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		draw,
		position,
		scale,
		rotation,
		rotationType,
		parent,
		destroyDiffuseRender,
		renderDiffuseCommand,
		diffuseRender);

	if (render == NULL)
	{
		free(diffuseRender);
		return NULL;
	}

	return render;
}

struct Mesh* getDiffuseRenderMesh(
	const struct Render* render)
{
	assert(render != NULL);
	struct DiffuseRender* diffuseRender =
		(struct DiffuseRender*)getRenderHandle(render);
	return diffuseRender->mesh;
}
void setDiffuseRenderMesh(
	struct Render* render,
	struct Mesh* mesh)
{
	assert(render != NULL);
	assert(mesh != NULL);

	struct DiffuseRender* diffuseRender =
		(struct DiffuseRender*)getRenderHandle(render);
	diffuseRender->mesh = mesh;
}

static void destroyTextRender(
	void* render)
{
	free(render);
}
static void renderTextCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Mat4F* model,
	const struct Mat4F* view,
	const struct Mat4F* proj,
	const struct Mat4F* viewProj,
	const struct Mat4F* mvp)
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
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	uint8_t rotationType,
	struct Render* parent,
	struct Vec4F color,
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
	 	rotationType,
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

struct Vec4F getTextRenderColor(
	const struct Render* render)
{
	assert(render != NULL);
	struct TextRender* textRender =
		(struct TextRender*)getRenderHandle(render);
	return textRender->color;
}
void setTextRenderColor(
	struct Render* render,
	struct Vec4F color)
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
