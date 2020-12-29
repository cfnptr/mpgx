#include "mpgx/ui.h"
#include <assert.h>

struct Ui
{
	struct Window* window;
	struct UiElement** elements;
	size_t elementCapacity;
	size_t elementCount;
};
struct UiElement
{
	struct Ui* ui;
	struct Transform* transform;
	bool update;
	DestroyUiElement destroyFunction;
	UiEvent cursorEnterFunction;
	UiEvent cursorExitFunction;
	UiEvent cursorStayFunction;
	UiEvent mousePressFunction;
	void* handle;
};

struct Ui* createUi(
	struct Window* window)
{
	assert(window != NULL);

	struct Ui* ui = malloc(
		sizeof(struct Ui));

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
	ui->elementCapacity = 1;
	ui->elementCount = 0;
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
	{
		struct UiElement* element = elements[i];
		element->destroyFunction(element->handle);
		free(element);
	}

	free(elements);
	free(ui);
}

struct UiElement* createUiElement(
	struct Ui* ui,
	struct Transform* transform,
	bool update,
	DestroyUiElement destroyFunction,
	UiEvent cursorEnterFunction,
	UiEvent cursorExitFunction,
	UiEvent cursorStayFunction,
	UiEvent mousePressFunction,
	void* handle)
{
	assert(ui != NULL);
	assert(transform != NULL);
	assert(destroyFunction != NULL);
	assert(cursorEnterFunction != NULL);
	assert(cursorExitFunction != NULL);
	assert(cursorStayFunction != NULL);
	assert(mousePressFunction != NULL);

	struct UiElement* element = malloc(
		sizeof(struct UiElement));

	if (element == NULL)
		return NULL;

	element->ui = ui;
	element->transform = transform;
	element->update = update;
	element->destroyFunction = destroyFunction;
	element->cursorEnterFunction = cursorEnterFunction;
	element->cursorExitFunction = cursorExitFunction;
	element->cursorStayFunction = cursorStayFunction;
	element->mousePressFunction = mousePressFunction;
	element->handle = handle;

	if (ui->elementCount == ui->elementCapacity)
	{
		size_t capacity =
			ui->elementCapacity * 2;
		struct UiElement** elements = realloc(
			ui->elements,
			capacity * sizeof(struct UiElement*));

		if (elements == NULL)
		{
			free(element);
			return NULL;
		}

		ui->elements = elements;
		ui->elementCapacity = capacity;
	}

	ui->elements[ui->elementCount] = element;
	ui->elementCount++;
	return element;
}
void destroyUiElement(
	struct UiElement* element)
{
	if (element == NULL)
		return;

	struct Ui* ui =
		element->ui;
	size_t elementCount =
		ui->elementCount;
	struct UiElement** elements =
		ui->elements;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (elements[i] == element)
		{
			for (size_t j = i + 1; j < elementCount; j++)
				elements[j - 1] = elements[j];

			element->destroyFunction(element->handle);
			ui->elementCount--;

			free(element);
			return;
		}
	}

	abort();
}

union Camera getUiCamera(
	const struct Ui* ui)
{
	assert(ui != NULL);

	struct Window* window =
		ui->window;

	size_t positionX, positionY;

	getWindowPosition(
		window,
		&positionX,
		&positionY);

	size_t width, height;

	getWindowSize(
		window,
		&width,
		&height);

	union Camera camera = createOrthographicCamera(
		(float)positionX,
		(float)positionX + width,
		(float)positionY,
		(float)positionY + height,
		0.0f,
		1.0f);
	return camera;
}

void executeUi(
	struct Ui* ui)
{

}
