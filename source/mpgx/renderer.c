#include "mpgx/renderer.h"

#include <assert.h>
#include <string.h>

struct Renderer
{
	struct Pipeline* pipeline;
	struct Transformer* transformer;
	bool ascendingSort;
	struct Transform* transform;
	struct Render** renders;
	size_t renderCapacity;
	size_t renderCount;
};
struct Render
{
	struct Renderer* renderer;
	bool draw;
	struct Transform* transform;
	struct Render* parent;
	DestroyRender destroyFunction;
	RenderCommand renderFunction;
	void* handle;
};

int ascendCompareRender(
	const void* a,
	const void* b)
{
	struct Render* render =
		*(struct Render**)a;
	struct Vector3F renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceA = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render =
		*(struct Render**)b;
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
		*(struct Render**)a;
	struct Vector3F renderPosition = addVector3F(
		getTransformPosition(render->transform),
		getTranslationMatrix4F(getTransformModel(render->transform)));
	float distanceA = distanceVector3F(
		getTransformPosition(render->renderer->transform),
		renderPosition);

	render =
		*(struct Render**)b;
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
	struct Pipeline* pipeline,
	struct Transformer* transformer,
	bool ascendingSort,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent)
{
	assert(pipeline != NULL);
	assert(transformer != NULL);

	struct Renderer* renderer = malloc(
		sizeof(struct Renderer));

	if (renderer == NULL)
		return NULL;

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

	renderer->pipeline = pipeline;
	renderer->transformer = transformer;
	renderer->ascendingSort = ascendingSort;
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

bool getRendererSorting(
	const struct Renderer* renderer)
{
	assert(renderer != NULL);
	return renderer->ascendingSort;
}
void setRendererSorting(
	struct Renderer* renderer,
	bool ascendingSorting)
{
	assert(renderer != NULL);
	renderer->ascendingSort = ascendingSorting;
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

	if (renderCount == 0)
		return;

	if (renderer->ascendingSort == true)
	{
		qsort(
			renders,
			renderCount,
			sizeof(struct Render*),
			ascendCompareRender);
	}
	else
	{
		qsort(
			renders,
			renderCount,
			sizeof(struct Render*),
			descendCompareRender);
	}

	struct Pipeline* pipeline =
		renderer->pipeline;
	uint8_t graphicsAPI = getWindowGraphicsAPI(
		getPipelineWindow(pipeline));

	struct Matrix4F view = getTransformModel(
		renderer->transform);

	struct Matrix4F proj;

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

	bindPipelineCommand(pipeline);

	for (size_t i = 0; i < renderCount; i++)
	{
		struct Render* render = renders[i];

		if (render->draw == false)
			continue;

		struct Render* parent =
			render->parent;

		while (parent != NULL)
		{
			if (parent->draw == false)
				goto CONTINUE;

			parent = parent->parent;
		}

		struct Matrix4F model = getTransformModel(
			render->transform);

		struct Matrix4F mvp = dotMatrix4F(
			view,
			proj);
		mvp = dotMatrix4F(
			model,
			mvp);

		render->renderFunction(
			render,
			pipeline,
			&model,
			&view,
			&proj,
			&mvp);

	CONTINUE:
		continue;
	}
}

struct Render* createRender(
	struct Renderer* renderer,
	bool draw,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Render* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle)
{
	assert(renderer != NULL);
	assert(destroyFunction != NULL);
	assert(renderFunction != NULL);

	struct Render* render = malloc(
		sizeof(struct Render));

	if (render == NULL)
		return NULL;

	struct Transform* transformParent;

	if (parent != NULL)
	{
		assert(renderer == parent->renderer);
		transformParent = parent->transform;
	}
	else
	{
		transformParent = NULL;
	}

	struct Transform* transform = createTransform(
		renderer->transformer,
		position,
		scale,
		rotation,
		transformParent);

	if (transform == NULL)
	{
		free(render);
		return NULL;
	}

	render->renderer = renderer;
	render->draw = draw;
	render->transform = transform;
	render->parent = parent;
	render->destroyFunction = destroyFunction;
	render->renderFunction = renderFunction;
	render->handle = handle;

	if (renderer->renderCount ==
		renderer->renderCapacity)
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
			free(render);

			renderer->renderCount--;
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
void* getRenderHandle(
	const struct Render* render)
{
	assert(render != NULL);
	return render->handle;
}

bool getRenderDraw(
	const struct Render* render)
{
	assert(render != NULL);
	return render->draw;
}
void setRenderDraw(
	struct Render* render,
	bool value)
{
	assert(render != NULL);
	render->draw = value;
}

struct Vector3F getRenderPosition(
	const struct Render* render)
{
	assert(render != NULL);

	return getTransformPosition(
		render->transform);
}
void setRenderPosition(
	struct Render* render,
	struct Vector3F position)
{
	assert(render != NULL);

	setTransformPosition(
		render->transform,
		position);
}

struct Vector3F getRenderScale(
	const struct Render* render)
{
	assert(render != NULL);

	return getTransformScale(
		render->transform);
}
void setRenderScale(
	struct Render* render,
	struct Vector3F scale)
{
	assert(render != NULL);

	setTransformScale(
		render->transform,
		scale);
}

struct Quaternion getRenderRotation(
	const struct Render* render)
{
	assert(render != NULL);

	return getTransformRotation(
		render->transform);
}
void setRenderRotation(
	struct Render* render,
	struct Quaternion rotation)
{
	assert(render != NULL);

	setTransformRotation(
		render->transform,
		rotation);
}

struct Render* getRenderParent(
	const struct Render* render)
{
	assert(render != NULL);
	return render->parent;
}
void setRenderParent(
	struct Render* render,
	struct Render* parent)
{
	assert(render != NULL);
	render->parent = parent;

	if (parent != NULL)
	{
		assert(render->renderer ==
			parent->renderer);

		setTransformParent(
			render->transform,
			parent->transform);
	}
	else
	{
		setTransformParent(
			render->transform,
			NULL);
	}
}
