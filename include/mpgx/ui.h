#pragma once
#include "mpgx/renderer.h"

struct Ui;
struct UiElement;
struct UiPanel;
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
	struct Window* window,
	struct Renderer* colorRenderer,
	struct Renderer* textRenderer);
void destroyUi(
	struct Ui* ui);
