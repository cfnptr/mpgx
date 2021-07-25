#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/texcol_pipeline.h"

Renderer createTexColRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity);
Render createTexColRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh);

Vec4F getTexColRenderColor(
	Render render);
void setTexColRenderColor(
	Render render,
	Vec4F color);

Vec2F getTexColRenderSize(
	Render render);
void setTexColRenderSize(
	Render render,
	Vec2F size);

Vec2F getTexColRenderOffset(
	Render render);
void setTexColRenderOffset(
	Render render,
	Vec2F offset);

Mesh getTexColRenderMesh(
	Render render);
void setTexColRenderMesh(
	Render render,
	Mesh mesh);