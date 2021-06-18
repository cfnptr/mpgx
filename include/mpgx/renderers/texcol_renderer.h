#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/texcol_pipeline.h"

Renderer* createTexColRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createTexColRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
	Mesh* mesh);

Vec4F getTexColRenderColor(
	const Render* render);
void setTexColRenderColor(
	Render* render,
	Vec4F color);

Vec2F getTexColRenderSize(
	const Render* render);
void setTexColRenderSize(
	Render* render,
	Vec2F size);

Vec2F getTexColRenderOffset(
	const Render* render);
void setTexColRenderOffset(
	Render* render,
	Vec2F offset);

Mesh* getTexColRenderMesh(
	const Render* render);
void setTexColRenderMesh(
	Render* render,
	Mesh* mesh);