#pragma once
#include "mpgx/renderer.h"

Renderer* createColorRenderer(
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSorting,
	Transform* parent);
Render* createColorRender(
	Renderer* renderer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	bool update,
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
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSorting,
	Transform* parent);
Render* createSpriteRender(
	Renderer* renderer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	bool update,
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
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSorting,
	Transform* parent);
Render* createDiffuseRender(
	Renderer* renderer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	bool update,
	Mesh* mesh);

Mesh* getDiffuseRenderMesh(
	const Render* render);
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh);

Renderer* createTextRenderer(
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSorting,
	Transform* parent);
Render* createTextRender(
	Renderer* renderer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	bool update,
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
