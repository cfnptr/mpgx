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
struct Render* createSpriteRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Vector4F color,
	struct Transform* parent,
	struct Mesh* mesh);
struct Render* createTextRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	struct Text* text);
