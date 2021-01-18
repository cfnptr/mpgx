#pragma once
#include "mpgx/renderer.h"

struct Ui;
struct UiElement;

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
	void* handle);
void destroyUiElement(
	struct UiElement* element);

union Camera getUiCamera(
	const struct Ui* ui);

void executeUi(
	struct Ui* ui);

struct UiElement* createUiButton(
	struct Ui* ui,
	struct Render* render,
	bool enabled,
	UiEvent cursorEnterFunction,
	UiEvent cursorExitFunction,
	UiEvent cursorStayFunction);
