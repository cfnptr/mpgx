#include "mpgx/renderer.h"
#include "mpgx/pipeline.h"

#include <assert.h>
#include <string.h>

typedef int(*CompareRender)(
	const void*,
	const void*);
typedef struct Matrix4F(*CreateProjMatrix)(
	union Camera);

struct Renderer
{
	struct Window* window;
	bool ascendingSort;
	enum CameraType cameraType;
	union Camera camera;
	struct Transform* transform;
	struct Pipeline* pipeline;
	CompareRender compareRenderFunction;
	CreateProjMatrix createProjFunction;
	struct Render** renders;
	struct Render** tmpRenders;
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
	struct Render* renderA =
		(struct Render*)a;
	float distanceA = distanceVector3F(
		getTransformPosition(renderA->renderer->transform),
		getTransformPosition(renderA->transform));

	struct Render* renderB =
		(struct Render*)b;
	float distanceB = distanceVector3F(
		getTransformPosition(renderB->renderer->transform),
		getTransformPosition(renderB->transform));

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
	struct Render* renderA =
		(struct Render*)a;
	float distanceA = distanceVector3F(
		getTransformPosition(renderA->renderer->transform),
		getTransformPosition(renderA->transform));

	struct Render* renderB =
		(struct Render*)b;
	float distanceB = distanceVector3F(
		getTransformPosition(renderB->renderer->transform),
		getTransformPosition(renderB->transform));

	if (distanceA > distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA < distanceB)
		return 1;

	abort();
}

struct Matrix4F createVkPerspectiveMatrix(
	union Camera camera)
{
	return createVkPerspectiveMatrix4F(
		camera.perspective.fieldOfView,
		camera.perspective.aspectRatio,
		camera.perspective.nearClipPlane,
		camera.perspective.farClipPlane);
}
struct Matrix4F createGlPerspectiveMatrix(
	union Camera camera)
{
	return createGlPerspectiveMatrix4F(
		camera.perspective.fieldOfView,
		camera.perspective.aspectRatio,
		camera.perspective.nearClipPlane,
		camera.perspective.farClipPlane);
}

struct Matrix4F createVkOrthographicMatrix(
	union Camera camera)
{
	return createVkOrthographicMatrix4F(
		camera.orthographic.leftFrustum,
		camera.orthographic.rightFrustum,
		camera.orthographic.bottomFrustum,
		camera.orthographic.topFrustum,
		camera.orthographic.nearClipPlane,
		camera.orthographic.farClipPlane);
}
struct Matrix4F createGlOrthographicMatrix(
	union Camera camera)
{
	return createGlOrthographicMatrix4F(
		camera.orthographic.leftFrustum,
		camera.orthographic.rightFrustum,
		camera.orthographic.bottomFrustum,
		camera.orthographic.topFrustum,
		camera.orthographic.nearClipPlane,
		camera.orthographic.farClipPlane);
}

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSort,
	enum CameraType cameraType,
	union Camera camera,
	struct Transform* transform,
	struct Pipeline* pipeline)
{
	assert(window != NULL);
	assert(transform != NULL);
	assert(pipeline != NULL);

	struct Renderer* renderer =
		malloc(sizeof(struct Renderer));

	if (renderer == NULL)
		return NULL;

	enum GraphicsAPI graphicsAPI =
		getWindowGraphicsAPI(window);

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

	if (cameraType == PERSPECTIVE_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			renderer->createProjFunction =
				createVkPerspectiveMatrix;
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			renderer->createProjFunction =
				createGlPerspectiveMatrix;
		}
		else
		{
			abort();
		}
	}
	else if (cameraType == ORTHOGRAPHIC_CAMERA_TYPE)
	{
		if (graphicsAPI == VULKAN_GRAPHICS_API)
		{
			renderer->createProjFunction =
				createVkOrthographicMatrix;
		}
		else if (graphicsAPI == OPENGL_GRAPHICS_API ||
			graphicsAPI == OPENGL_ES_GRAPHICS_API)
		{
			renderer->createProjFunction =
				createGlOrthographicMatrix;
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

	struct Render** renders = malloc(
		sizeof(struct Render*));

	if (renders == NULL)
	{
		free(renderer);
		return NULL;
	}

	struct Render** tmpRenders = malloc(
		sizeof(struct Render*));

	if (tmpRenders == NULL)
	{
		free(renders);
		free(renderer);
		return NULL;
	}

	renderer->window = window;
	renderer->ascendingSort = ascendingSort;
	renderer->cameraType = cameraType;
	renderer->camera = camera;
	renderer->transform = transform;
	renderer->pipeline = pipeline;
	renderer->renders = renders;
	renderer->tmpRenders = tmpRenders;
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

	free(renderer->tmpRenders);
	free(renders);
	free(renderer);
}

struct Window* getRendererWindow(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->window;
}
bool getRendererAscendingSort(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->ascendingSort;
}
enum CameraType getRendererCameraType(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->cameraType;
}
struct Transform* getRendererTransformer(
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

union Camera getRendererCamera(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->camera;
}
void setRendererCamera(
	struct Renderer* renderer,
	union Camera camera)
{
	assert(renderer != NULL);
	renderer->camera = camera;
}

void executeRenderer(
	struct Renderer* renderer)
{
	assert(renderer != NULL);

	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;
	struct Render** tmpRenders =
		renderer->tmpRenders;
	struct Pipeline* pipeline =
		renderer->pipeline;

	memcpy(
		tmpRenders,
		renders,
		renderCount * sizeof(struct Render*));

	qsort(
		tmpRenders,
		renderCount,
		sizeof(struct Render*),
		renderer->compareRenderFunction);

	struct Matrix4F view = getTransformModel(
		renderer->transform);

	CreateProjMatrix createProjFunction =
		renderer->createProjFunction;
	struct Matrix4F proj =
		createProjFunction(renderer->camera);

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

			RenderCommand renderFunction =
				render->renderFunction;

			renderFunction(
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
	struct Transform* transform,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle)
{
	assert(renderer != NULL);
	assert(transform != NULL);
	assert(destroyFunction != NULL);
	assert(renderFunction != NULL);

	struct Render* render =
		malloc(sizeof(struct Render));

	if (render == NULL)
		return NULL;

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
			free(render);
			return NULL;
		}

		renderer->renders = renders;

		struct Render** tmpRenders = realloc(
			renderer->renders,
			capacity * sizeof(struct Render*));

		if (tmpRenders == NULL)
		{
			free(render);
			return NULL;
		}

		renderer->tmpRenders = tmpRenders;
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
	assert(render->destroyFunction != NULL);

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
	if (render == NULL)
		return;

	struct MeshRender* textRender =
		(struct MeshRender*)render;
	free(textRender);
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
	struct Transform* transform,
	struct Mesh* mesh)
{
	assert(renderer != NULL);
	assert(transform != NULL);
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
		transform,
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

void destroyTextRender(
	void* render)
{
	if (render == NULL)
		return;

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
	struct Transform* transform,
	struct Text* text)
{
	assert(renderer != NULL);
	assert(transform != NULL);
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
		transform,
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
