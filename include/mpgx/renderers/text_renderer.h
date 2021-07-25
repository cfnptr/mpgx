#pragma once
#include "mpgx/renderer.h"
#include "mpgx/text.h"

Renderer createTextRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity);
Render createTextRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Text text);

Vec4F getTextRenderColor(
	Render render);
void setTextRenderColor(
	Render render,
	Vec4F color);

Text getTextRenderText(
	Render render);
void setTextRenderText(
	Render render,
	Text text);
