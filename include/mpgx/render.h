#pragma once
#include "mpgx/renderer.h"

Renderer* createColorRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createColorRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Mesh* mesh);

Vec4F getColorRenderColor(
	const Render* render);
void setColorRenderColor(
	Render* render,
	Vec4F color);

Mesh* getColorRenderMesh(
	const Render* render);
void setColorRenderMesh(
	Render* render,
	Mesh* mesh);

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

Renderer* createSpriteRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createSpriteRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Mesh* mesh);

Vec4F getSpriteRenderColor(
	const Render* render);
void setSpriteRenderColor(
	Render* render,
	Vec4F color);

Mesh* getSpriteRenderMesh(
	const Render* render);
void setSpriteRenderMesh(
	Render* render,
	Mesh* mesh);

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

Renderer* createDiffuseRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createDiffuseRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Mesh* mesh);

Mesh* getDiffuseRenderMesh(
	const Render* render);
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh);

Renderer* createTextRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createTextRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Vec4F color,
	Text* text);

Vec4F getTextRenderColor(
	const Render* render);
void setTextRenderColor(
	Render* render,
	Vec4F color);

Text* getTextRenderText(
	const Render* render);
void setTextRenderText(
	Render* render,
	Text* text);
