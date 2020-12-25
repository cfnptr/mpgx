#include "mpgx/renderer.h"

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
		assert(renders[i]->destroyFunction != NULL);

		DestroyRender destroyFunction =
			renders[i]->destroyFunction;
		destroyFunction(renders[i]);
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

int compareRender(
	const void* a,
	const void* b)
{
	if (*(struct Render**)a <
		*(struct Render**)b)
	{
		return -1;
	}
	if (*(struct Render**)a ==
		*(struct Render**)b)
	{
		return 0;
	}
	if (*(struct Render**)a >
		*(struct Render**)b)
	{
		return 1;
	}

	abort();
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

	render->renderer = renderer;
	render->render = _render;
	render->transform = transform;
	render->destroyFunction = destroyFunction;
	render->renderFunction = renderFunction;
	render->handle = handle;

	renderer->renders[
		renderer->renderCount] = render;
	renderer->renderCount++;

	qsort(
		renderer->renders,
		renderer->renderCount,
		sizeof(struct Render*),
		compareRender);

	return render;
}
void destroyRender(
	struct Render* _render)
{
	assert(_render->destroyFunction != NULL);

	if (_render == NULL)
		return;

	struct Renderer* renderer =
		_render->renderer;
	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;

	struct Render** render = bsearch(
		&_render,
		renders,
		renderCount,
		sizeof(struct Render*),
		compareRender);

	if (render == NULL)
		abort();

	size_t index = render - renders;

	for (size_t j = index + 1; j < renderCount; j++)
		renders[j - 1] = renders[j];

	renderer->renderCount--;

	DestroyRender destroyFunction =
		_render->destroyFunction;
	destroyFunction(_render);

	free(_render);
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

void destroyTextRender(
	struct Render* render)
{
	if (render == NULL)
		return;

	struct TextRender* textRender =
		(struct TextRender*)render->handle;
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