#include "mpgx/renderer.h"
#include "mpgx/pipeline.h"

#include <assert.h>
#include <string.h>

typedef int(*CompareRender)(
	const void*,
	const void*);

struct Renderer
{
	struct Window* window;
	bool ascendingSort;
	struct Pipeline* pipeline;
	struct Transformer* transformer;
	struct Transform* transform;
	CompareRender compareRenderFunction;
	struct Render** renders;
	size_t renderCapacity;
	size_t renderCount;
};
struct Render
{
	struct Renderer* renderer;
	bool render;
	struct Transform* transform;
	DestroyRender destroyFunction;
	RenderCommand renderFunction;
	void* handle;
};

struct MeshRender
{
	struct Mesh* mesh;
};
struct TextRender
{
	struct Text* text;
};

int ascendCompareRender(
	const void* a,
	const void* b)
{
	struct Render* render =
		(struct Render*)a;
	struct Vector3F renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceA = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render =
		(struct Render*)b;
	renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceB = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	if (distanceA < distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA > distanceB)
		return 1;

	abort();
}
int descendCompareRender(
	const void* a,
	const void* b)
{
	struct Render* render =
		(struct Render*)a;
	struct Vector3F renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceA = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render =
		(struct Render*)b;
	renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceB = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	if (distanceA > distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA < distanceB)
		return 1;

	abort();
}

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSort,
	struct Pipeline* pipeline,
	struct Transformer* transformer,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent)
{
	assert(window != NULL);
	assert(pipeline != NULL);
	assert(transformer != NULL);

	struct Renderer* renderer =
		malloc(sizeof(struct Renderer));

	if (renderer == NULL)
		return NULL;

	if (ascendingSort == true)
	{
		renderer->compareRenderFunction =
			ascendCompareRender;
	}
	else
	{
		renderer->compareRenderFunction =
			descendCompareRender;
	}

	struct Transform* transform = createTransform(
		transformer,
		position,
		scale,
		rotation,
		parent);

	if (transform == NULL)
	{
		free(renderer);
		return NULL;
	}

	struct Render** renders = malloc(
		sizeof(struct Render*));

	if (renders == NULL)
	{
		destroyTransform(transform);
		free(renderer);
		return NULL;
	}

	renderer->window = window;
	renderer->ascendingSort = ascendingSort;
	renderer->pipeline = pipeline;
	renderer->transformer = transformer;
	renderer->transform = transform;
	renderer->renders = renders;
	renderer->renderCapacity = 1;
	renderer->renderCount = 0;
	return renderer;
}
void destroyRenderer(
	struct Renderer* renderer)
{
	if (renderer == NULL)
		return;

	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;

	for (size_t i = 0; i < renderCount; i++)
	{
		struct Render* render = renders[i];
		render->destroyFunction(render->handle);
		free(render);
	}

	free(renders);
	free(renderer);
}

struct Window* getRendererWindow(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->window;
}
bool isRendererAscendingSort(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->ascendingSort;
}
struct Transformer* getRendererTransformer(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->transformer;
}
struct Transform* getRendererTransform(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->transform;
}
struct Pipeline* getRendererPipeline(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->pipeline;
}

void executeRenderer(
	struct Renderer* renderer,
	union Camera camera)
{
	assert(renderer != NULL);

	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;
	struct Pipeline* pipeline =
		renderer->pipeline;

	qsort(
		renders,
		renderCount,
		sizeof(struct Render*),
		renderer->compareRenderFunction);

	struct Matrix4F view = getTransformModel(
		renderer->transform);

	struct Matrix4F proj;

	uint8_t graphicsAPI = getWindowGraphicsAPI(
		renderer->window);

	if (camera.perspective.type == PERSPECTIVE_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = createVkPerspectiveMatrix4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = createGlPerspectiveMatrix4F(
				camera.perspective.fieldOfView,
				camera.perspective.aspectRatio,
				camera.perspective.nearClipPlane,
				camera.perspective.farClipPlane);
		}
		else
		{
			abort();
		}
	}
	else if (camera.orthographic.type == ORTHOGRAPHIC_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			proj = createVkOrthographicMatrix4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			proj = createGlOrthographicMatrix4F(
				camera.orthographic.leftFrustum,
				camera.orthographic.rightFrustum,
				camera.orthographic.bottomFrustum,
				camera.orthographic.topFrustum,
				camera.orthographic.nearClipPlane,
				camera.orthographic.farClipPlane);
		}
		else
		{
			abort();
		}
	}
	else
	{
		abort();
	}

	bindPipelineCommand(
		renderer->pipeline);

	for (size_t i = 0; i < renderCount; i++)
	{
		struct Render* render = renders[i];

		if (render->render == true)
		{
			assert(render->transform != NULL);
			assert(render->renderFunction != NULL);

			struct Matrix4F model = getTransformModel(
				render->transform);

			struct Matrix4F mvp = mulMatrix4F(
				proj,
				view);
			mvp = mulMatrix4F(
				mvp,
				model);

			render->renderFunction(
				render,
				pipeline,
				&model,
				&view,
				&proj,
				&mvp);
		}
	}
}

struct Render* createRender(
	struct Renderer* renderer,
	bool _render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle)
{
	assert(renderer != NULL);
	assert(destroyFunction != NULL);
	assert(renderFunction != NULL);

	struct Render* render =
		malloc(sizeof(struct Render));

	if (render == NULL)
		return NULL;

	struct Transform* transform = createTransform(
		renderer->transformer,
		position,
		scale,
		rotation,
		parent);

	if (transform == NULL)
	{
		free(render);
		return NULL;
	}

	render->renderer = renderer;
	render->render = _render;
	render->transform = transform;
	render->destroyFunction = destroyFunction;
	render->renderFunction = renderFunction;
	render->handle = handle;

	if (renderer->renderCount == renderer->renderCapacity)
	{
		size_t capacity =
			renderer->renderCapacity * 2;
		struct Render** renders = realloc(
			renderer->renders,
			capacity * sizeof(struct Render*));

		if (renders == NULL)
		{
			destroyTransform(transform);
			free(render);
			return NULL;
		}

		renderer->renders = renders;
		renderer->renderCapacity = capacity;
	}

	renderer->renders[
		renderer->renderCount] = render;
	renderer->renderCount++;
	return render;
}
void destroyRender(
	struct Render* render)
{
	if (render == NULL)
		return;

	struct Renderer* renderer =
		render->renderer;
	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;

	for (size_t i = 0; i < renderCount; i++)
	{
		if (renders[i] == render)
		{
			for (size_t j = i + 1; j < renderCount; j++)
				renders[j - 1] = renders[j];

			render->destroyFunction(render->handle);
			destroyTransform(render->transform);

			renderer->renderCount--;
			free(render);
			return;
		}
	}

	abort();
}

struct Renderer* getRenderRenderer(
	const struct Render* render)
{
	assert(render != NULL);
	return render->renderer;
}
struct Transform* getRenderTransform(
	const struct Render* render)
{
	assert(render != NULL);
	return render->transform;
}
struct Renderer* getRenderHandle(
	const struct Render* render)
{
	assert(render != NULL);
	return render->handle;
}

bool getRenderRender(
	const struct Render* render)
{
	assert(render != NULL);
	return render->render;
}
void setRenderRender(
	struct Render* render,
	bool value)
{
	assert(render != NULL);
	render->render = value;
}

void destroyMeshRender(
	void* render)
{
	struct MeshRender* meshRender =
		(struct MeshRender*)render;
	free(meshRender);
}

void renderColorCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp)
{
	struct MeshRender* meshRender =
		(struct MeshRender*)render->handle;

	setColorPipelineMVP(
		pipeline,
		*mvp);
	drawMeshCommand(
		meshRender->mesh,
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
	assert(renderer->window == getMeshWindow(mesh));

	struct MeshRender* meshRender = malloc(
		sizeof(struct MeshRender));

	if (meshRender == NULL)
		return NULL;

	meshRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		_render,
		position,
		scale,
		rotation,
		parent,
		destroyMeshRender,
		renderColorCommand,
		meshRender);

	if (render == NULL)
	{
		free(meshRender);
		return NULL;
	}

	return render;
}

void renderSpriteCommand(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp)
{
	struct MeshRender* meshRender =
		(struct MeshRender*)render->handle;

	setSpritePipelineMVP(
		pipeline,
		*mvp);
	drawMeshCommand(
		meshRender->mesh,
		pipeline);
}
struct Render* createSpriteRender(
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
	assert(renderer->window == getMeshWindow(mesh));

	struct MeshRender* meshRender = malloc(
		sizeof(struct MeshRender));

	if (meshRender == NULL)
		return NULL;

	meshRender->mesh = mesh;

	struct Render* render = createRender(
		renderer,
		_render,
		position,
		scale,
		rotation,
		parent,
		destroyMeshRender,
		renderSpriteCommand,
		meshRender);

	if (render == NULL)
	{
		free(meshRender);
		return NULL;
	}

	return render;
}

void destroyTextRender(
	void* render)
{
	struct TextRender* textRender =
		(struct TextRender*)render;
	free(textRender);
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
		(struct TextRender*)render->handle;

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
	assert(renderer->window == getTextWindow(text));

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
