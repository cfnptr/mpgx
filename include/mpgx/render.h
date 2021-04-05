#pragma once
#include "mpgx/renderer.h"

struct Render* createColorRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Render* parent,
	struct Vec4F color,
	struct Mesh* mesh);

struct Mesh* getColorRenderMesh(
	const struct Render* render);
void setColorRenderMesh(
	struct Render* render,
	struct Mesh* mesh);

struct Vec4F getColorRenderColor(
	const struct Render* render);
void setColorRenderColor(
	struct Render* render,
	struct Vec4F color);

struct Render* createSpriteRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Render* parent,
	struct Vec4F color,
	struct Mesh* mesh);

struct Vec4F getSpriteRenderColor(
	const struct Render* render);
void setSpriteRenderColor(
	struct Render* render,
	struct Vec4F color);

struct Mesh* getSpriteRenderMesh(
	const struct Render* render);
void setSpriteRenderMesh(
	struct Render* render,
	struct Mesh* mesh);

struct Render* createDiffuseRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Render* parent,
	struct Mesh* mesh);

struct Mesh* getDiffuseRenderMesh(
	const struct Render* render);
void setDiffuseRenderMesh(
	struct Render* render,
	struct Mesh* mesh);

struct Render* createTextRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Render* parent,
	struct Vec4F color,
	struct Text* text);

struct Vec4F getTextRenderColor(
	const struct Render* render);
void setTextRenderColor(
	struct Render* render,
	struct Vec4F color);

struct Text* getTextRenderText(
	const struct Render* render);
void setTextRenderText(
	struct Render* render,
	struct Text* text);
