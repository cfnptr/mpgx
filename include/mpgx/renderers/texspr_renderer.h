#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/texspr_pipeline.h"

Renderer createTexSprRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity);
Render createTexSprRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh);

Vec4F getTexSprRenderColor(
	Render render);
void setTexSprRenderColor(
	Render render,
	Vec4F color);

Vec2F getTexSprRenderSize(
	Render render);
void setTexSprRenderSize(
	Render render,
	Vec2F size);

Vec2F getTexSprRenderOffset(
	Render render);
void setTexSprRenderOffset(
	Render render,
	Vec2F offset);

Mesh getTexSprRenderMesh(
	Render render);
void setTexSprRenderMesh(
	Render render,
	Mesh mesh);
