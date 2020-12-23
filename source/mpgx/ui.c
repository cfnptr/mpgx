#include "mpgx/ui.h"
#include "mpgx/camera.h"

#include <string.h>
#include <assert.h>

struct Ui
{
	struct Window* window;
	struct UiElement** elements;
	size_t elementCount;
	size_t elementCapacity;
};
struct UiElement
{
	struct Ui* ui;
	struct Pipeline* pipeline;
	struct Mesh* mesh;
	struct Vector3F position;
	struct Vector3F scale;
	// bake matrix and the multiply by parents
	struct Matrix4F model;
	// TODO: quaternion
	bool draw;
	struct UiElement* parent;
};

struct Ui* createUi(
	struct Window* window)
{
	assert(window != NULL);

	struct Ui* ui =
		malloc(sizeof(struct Ui));

	if (ui == NULL)
		return NULL;

	struct UiElement** elements = malloc(
		sizeof(struct UiElement*));

	if (elements == NULL)
	{
		free(ui);
		return NULL;
	}

	ui->window = window;
	ui->elements = elements;
	ui->elementCount = 0;
	ui->elementCapacity = 1;
	return ui;
}
void destroyUi(
	struct Ui* ui)
{
	if (ui == NULL)
		return;

	size_t elementCount =
		ui->elementCount;
	struct UiElement** elements =
		ui->elements;

	for (size_t i = 0; i < elementCount; i++)
		free(elements[i]);

	free(elements);
	free(ui);
}

struct UiElement* createUiElement(
	struct Ui* ui,
	struct Pipeline* pipeline,
	struct Mesh* mesh,
	struct Vector3F position,
	struct Vector3F scale,
	bool draw,
	struct UiElement* parent)
{
	assert(ui != NULL);
	assert(pipeline != NULL);
	assert(mesh != NULL);
	assert(ui->window == getPipelineWindow(pipeline));
	assert(ui->window == getMeshWindow(mesh));

	struct UiElement* uiElement =
		malloc(sizeof(struct UiElement));

	if (uiElement == NULL)
		return NULL;

	if (ui->elementCount == ui->elementCapacity)
	{
		size_t capacity =
			ui->elementCapacity * 2;
		struct UiElement** elements = realloc(
			ui->elements,
			capacity * sizeof(struct UiElement*));

		if (elements == NULL)
		{
			free(uiElement);
			return NULL;
		}

		ui->elements = elements;
		ui->elementCapacity = capacity;
	}

	ui->elements[ui->elementCount] = uiElement;
	ui->elementCount++;

	uiElement->ui = ui;
	uiElement->pipeline = pipeline;
	uiElement->mesh = mesh;
	uiElement->position = position;
	uiElement->scale = scale;
	uiElement->draw = draw;
	uiElement->model = createIdentityMatrix4F();
	uiElement->parent = parent;
	return uiElement;
}
void destroyUiElement(
	struct UiElement* uiElement)
{
	if (uiElement == NULL)
		return;

	struct Ui* ui =
		uiElement->ui;
	size_t uiElementCount =
		ui->elementCount;
	struct UiElement** uiElements =
		ui->elements;

	for (size_t i = 0; i < uiElementCount; i++)
	{
		if (uiElement == uiElements[i])
		{
			for (size_t j = i + 1; j < uiElementCount; j++)
				uiElements[j - 1] = uiElements[j];

			ui->elementCount--;
			free(uiElement);
			return;
		}
	}

	abort();
}


int compareUiElement(
	const void* a,
	const void* b)
{
	if (((struct UiElement*)a)->position.z >
		((struct UiElement*)b)->position.z)
	{
		return -1;
	}
	if (((struct UiElement*)a)->position.z ==
		((struct UiElement*)b)->position.z)
	{
		return 0;
	}
	if (((struct UiElement*)a)->position.z <
		((struct UiElement*)b)->position.z)
	{
		return 1;
	}

	abort();
}
void updateUi(
	struct Ui* ui)
{
	assert(ui != NULL);

	size_t uiElementsCount =
		ui->elementCount;
	struct UiElement** uiElements =
		ui->elements;

	qsort(
		uiElements,
		uiElementsCount,
		sizeof(struct UiElement*),
		compareUiElement);

	size_t windowWidth;
	size_t windowHeight;

	getWindowSize(
		ui->window,
		&windowWidth,
		&windowHeight);

	size_t positionX;
	size_t positionY;

	getWindowSize(
		ui->window,
		&positionX,
		&positionY);

	for (size_t i = 0; i < uiElementsCount; i++)
	{
		struct UiElement* uiElement =
			uiElements[i];

		struct Matrix4F model =
			createIdentityMatrix4F();
		model = mulMatrix4F(model,
			scaleMatrix4F(model, uiElement->scale));
		model = mulMatrix4F(model,
			translateMatrix4F(model, uiElement->position));
		uiElement->model = model;
	}

	struct OrthographicCamera camera = createOrthographicCamera(
		(float)positionX,
		(float)positionX + windowWidth,
		(float)positionY,
		(float)positionY + windowHeight,
		0.0f,
		1.0f);

	struct Pipeline* lastPipeline = NULL;

	for (size_t i = 0; i < uiElementsCount; i++)
	{
		struct UiElement* uiElement =
			uiElements[i];

		if (uiElement->draw)
		{
			if (uiElement->pipeline != lastPipeline)
			{
				lastPipeline = uiElement->pipeline;
				bindPipelineCommand(lastPipeline);
			}

			// TODO: create pipeline setuniform methos
			// and set uniforms

			drawMeshCommand(
				uiElement->mesh,
				lastPipeline);
		}
	}
}
