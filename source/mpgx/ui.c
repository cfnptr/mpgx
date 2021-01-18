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
	void* handle;
};
struct UiPanel
{
	struct Render* render;
};
struct UiButton
{
	bool enabled;
	struct Render* defaultRender;
	struct Render* highlightedRender;
	struct Render* pressedRender;
	struct Render* disabledRender;
};
//struct UiImage;
//struct UiText;
//struct UiTextField;
//struct UiButton;
//struct UiRadioButton;
//struct UiCheckbox;
//struct UiToggle;
//struct UiSlider;
// TODO: other ui elements

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
	void* handle)
{
	assert(ui != NULL);
	assert(transform != NULL);
	assert(destroyFunction != NULL);
	assert(cursorEnterFunction != NULL);
	assert(cursorExitFunction != NULL);
	assert(cursorStayFunction != NULL);

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

	size_t width, height;

	getWindowSize(
		window,
		&width,
		&height);

	union Camera camera = createOrthographicCamera(
		0.0f,
		(float)width,
		0.0f,
		(float)height,
		0.0f,
		1.0f);
	return camera;
}

void executeUi(
	struct Ui* ui)
{
	assert(ui != NULL);

	double cursorX, cursorY;

	getWindowCursorPosition(
		ui->window,
		&cursorX,
		&cursorY);

	size_t elementCount = ui->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (ui->elements[i]->update == false)
			continue;

		struct UiElement* element = ui->elements[i];


	}
}

void destroyUiPanel(void* panel)
{
	struct UiPanel* uiPanel =
		(struct UiPanel*)panel;
	free(uiPanel);
}
// TODO:
struct UiElement* createUiButton(
	struct Ui* ui,
	struct Render* render,
	bool enabled,
	UiEvent cursorEnterFunction,
	UiEvent cursorExitFunction,
	UiEvent cursorStayFunction)
{
	assert(ui != NULL);
	assert(render != NULL);

	assert(getRendererWindow(
		getRenderRenderer(render)) == ui->window);

	struct UiPanel* panel = malloc(
		sizeof(struct UiPanel));

	if (panel == NULL)
		return NULL;

	panel->render = render;

	struct UiElement* element = createUiElement(
		ui,
		getRenderTransform(render),
		enabled,
		destroyUiPanel,
		cursorEnterFunction,
		cursorExitFunction,
		cursorStayFunction,
		panel);

	if (element == NULL)
	{
		free(panel);
		return NULL;
	}

	return element;
}
