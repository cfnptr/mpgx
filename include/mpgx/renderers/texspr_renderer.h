#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/texspr_pipeline.h"

Renderer* createTexSprRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createTexSprRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
	Mesh* mesh);

Vec4F getTexSprRenderColor(
	const Render* render);
void setTexSprRenderColor(
	Render* render,
	Vec4F color);

Vec2F getTexSprRenderSize(
	const Render* render);
void setTexSprRenderSize(
	Render* render,
	Vec2F size);

Vec2F getTexSprRenderOffset(
	const Render* render);
void setTexSprRenderOffset(
	Render* render,
	Vec2F offset);

Mesh* getTexSprRenderMesh(
	const Render* render);
void setTexSprRenderMesh(
	Render* render,
	Mesh* mesh);
