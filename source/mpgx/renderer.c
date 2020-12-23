#include "mpgx/renderer.h"

#include <assert.h>

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
	struct Vector3F position;
	struct Vector3F scale;
	struct Quaternion rotation;
	CompareRender compareRenderFunction;
	CreateProjMatrix createProjFunction;
	struct Render** renders;
	size_t renderCapacity;
	size_t renderCount;
};

int ascendCompareRender(
	const void* a,
	const void* b)
{
	struct Render* renderA =
		(struct Render*)a;
	float distanceA = distanceVector3F(
		renderA->renderer->position,
		renderA->position);

	struct Render* renderB =
		(struct Render*)b;
	float distanceB = distanceVector3F(
		renderB->renderer->position,
		renderB->position);

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
		renderA->renderer->position,
		renderA->position);

	struct Render* renderB =
		(struct Render*)b;
	float distanceB = distanceVector3F(
		renderB->renderer->position,
		renderB->position);

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
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation)
{
	assert(window != NULL);

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

	renderer->window = window;
	renderer->ascendingSort = ascendingSort;
	renderer->cameraType = cameraType;
	renderer->camera = camera;
	renderer->position = position;
	renderer->scale = scale;
	renderer->rotation = rotation;
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
		assert(renders[i]->destroyFunction != NULL);

		DestroyRender destroyFunction =
			renders[i]->destroyFunction;
		destroyFunction(renders[i]);
	}

	free(renders);
	free(renderer);
}

struct Render* createRender(
	struct Renderer* renderer,
	bool _render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Matrix4F model,
	struct Render* parent,
	DestroyRender destroyFunction,
	DrawRenderCommand drawFunction)
{
	assert(renderer != NULL);
	assert(destroyFunction != NULL);
	assert(drawFunction != NULL);

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
		renderer->renderCapacity = capacity;
	}

	render->renderer = renderer;
	render->render = _render;
	render->position = position;
	render->scale = scale;
	render->rotation = rotation;
	render->model = model;
	render->parent = parent;
	render->destroyFunction = destroyFunction;
	render->drawFunction = drawFunction;

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
		if (render == renders[i])
		{
			for (size_t j = i + 1; j < renderCount; j++)
				renders[j - 1] = renders[j];

			renderer->renderCount--;

			DestroyRender destroyFunction =
				render->destroyFunction;
			destroyFunction(render);

			return;
		}
	}

	abort();
}

void drawRenderer(
	struct Renderer* renderer)
{
	assert(renderer != NULL);

	size_t renderCount =
		renderer->renderCount;
	struct Render** renders =
		renderer->renders;
	CompareRender compareRender =
		renderer->compareRenderFunction;

	qsort(
		renders,
		renderCount,
		sizeof(struct Render*),
		compareRender);

	for (size_t i = 0; i < renderCount; i++)
	{
		struct Render* render = renders[i];

		struct Matrix4F model =
			createIdentityMatrix4F();
		model = mulMatrix4F(model,
			translateMatrix4F(model, render->position));
		model = mulMatrix4F(model,
			getQuaternionMatrixF4(render->rotation));
		model = mulMatrix4F(model,
			scaleMatrix4F(model, render->scale));
		render->model = model;
	}

	// TODO:
	// multiply by parent matrices
	// for ...

	struct Matrix4F view =
		createIdentityMatrix4F();
	view = mulMatrix4F(view,
		translateMatrix4F(view, renderer->position));
	view = mulMatrix4F(view,
		getQuaternionMatrixF4(renderer->rotation));
	view = mulMatrix4F(view,
		scaleMatrix4F(view, renderer->scale));

	CreateProjMatrix createProjFunction =
		renderer->createProjFunction;
	struct Matrix4F proj =
		createProjFunction(renderer->camera);

	for (size_t i = 0; i < renderCount; i++)
	{
		struct Render* render = renders[i];

		if (render->render)
		{
			struct Matrix4F mvp = mulMatrix4F(
				proj,
				view);
			mvp = mulMatrix4F(
				mvp,
				render->model);

			assert(render->drawFunction != NULL);

			DrawRenderCommand drawFunction =
				render->drawFunction;

			drawFunction(
				render,
				&render->model,
				&view,
				&proj,
				&mvp);
		}
	}
}
