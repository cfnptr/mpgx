#pragma once
#include "mpgx/window.h"

struct Ui;
struct UiElement;

struct Ui* createUi(
	struct Window* window);
void destroyUi(
	struct Ui* ui);

struct UiElement* createUiElement(
	struct Ui* ui,
	struct Pipeline* pipeline,
	struct Mesh* mesh,
	struct Vector3F position,
	struct Vector3F scale,
	bool draw,
	struct UiElement* parent);
void destroyUiElement(
	struct UiElement* uiElement);

void updateUi(
	struct Ui* ui);
