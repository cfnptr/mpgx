#pragma once
#include "mpgx/renderer.h"

struct Render* createColorRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	struct Mesh* mesh);
struct Mesh* getColorRenderMesh(
	const struct Render* render);

struct Render* createSpriteRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Vector4F color,
	struct Transform* parent,
	struct Mesh* mesh);
struct Mesh* getSpriteRenderMesh(
	const struct Render* render);
struct Vector4F getSpriteRenderColor(
	const struct Render* render);

struct Render* createTextRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	struct Text* text);
struct Text* getTextRenderText(
	const struct Render* render);
