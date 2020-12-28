#pragma once
#include "mpgx/camera.h"
#include "mpgx/window.h"
#include "mpgx/transformer.h"

struct Ui;
struct UiElement;
//struct UiPanel;
//struct UiImage;
//struct UiText;
//struct UiTextField;
//struct UiButton;
//struct UiRadioButton;
//struct UiCheckbox;
//struct UiToggle;
//struct UiSlider;
// TODO: other ui elements

typedef void(*DestroyUiElement)(void*);
typedef void(*UiEvent)(void*);

struct Ui* createUi(
	struct Window* window);
void destroyUi(
	struct Ui* ui);

struct UiElement* createUiElement(
	struct Ui* ui,
	struct Transform* transform,
	bool update,
	DestroyUiElement destroyFunction,
	UiEvent cursorEnterFunction,
	UiEvent cursorExitFunction,
	UiEvent cursorStayFunction,
	UiEvent mousePressFunction,
	void* handle);
void destroyUiElement(
	struct UiElement* element);

union Camera getUiCamera(
	const struct Ui* ui);

void executeUi(
	struct Ui* ui);
