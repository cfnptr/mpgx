#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/color_pipeline.h"

Renderer createColorRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType);
Render createColorRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Mesh mesh);

Vec4F getColorRenderColor(
	Render render);
void setColorRenderColor(
	Render render,
	Vec4F color);

Mesh getColorRenderMesh(
	Render render);
void setColorRenderMesh(
	Render render,
	Mesh mesh);
