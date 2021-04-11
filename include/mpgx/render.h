#pragma once
#include "mpgx/renderer.h"

Renderer* createColorRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting);
Render* createColorRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
	Mesh* mesh);

Mesh* getColorRenderMesh(
	const Render* render);
void setColorRenderMesh(
	Render* render,
	Mesh* mesh);

Vector4F getColorRenderColor(
	const Render* render);
void setColorRenderColor(
	Render* render,
	Vector4F color);

Renderer* createSpriteRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting);
Render* createSpriteRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
	Mesh* mesh);

Vector4F getSpriteRenderColor(
	const Render* render);
void setSpriteRenderColor(
	Render* render,
	Vector4F color);

Mesh* getSpriteRenderMesh(
	const Render* render);
void setSpriteRenderMesh(
	Render* render,
	Mesh* mesh);

Renderer* createDiffuseRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting);
Render* createDiffuseRender(
	Renderer* renderer,
	Transform* transform,
	Mesh* mesh);

Mesh* getDiffuseRenderMesh(
	const Render* render);
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh);

Renderer* createTextRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting);
Render* createTextRender(
	Renderer* renderer,
	Transform* transform,
	Vector4F color,
	Text* text);

Vector4F getTextRenderColor(
	const Render* render);
void setTextRenderColor(
	Render* render,
	Vector4F color);

Text* getTextRenderText(
	const Render* render);
void setTextRenderText(
	Render* render,
	Text* text);
