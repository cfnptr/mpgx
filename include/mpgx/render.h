#pragma once
#include "mpgx/renderer.h"

Render* createColorRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
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

Render* createSpriteRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
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

Render* createDiffuseRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	Mesh* mesh);

Mesh* getDiffuseRenderMesh(
	const Render* render);
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh);

Render* createTextRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
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
