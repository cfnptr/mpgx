#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/quaternion.h"
#include "mpgx/text.h"

struct Renderer;
struct Render;

typedef void(*DestroyRender)(
	struct Render*);
typedef void(*RenderCommand)(
	struct Render*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*);

struct Render
{
	struct Renderer* renderer;
	bool render;
	struct Vector3F position;
	struct Vector3F scale;
	struct Matrix4F model;
	struct Quaternion rotation;
	struct Render* parent;
	DestroyRender destroyFunction;
	RenderCommand renderFunction;
	void* handle;
};

// TODO:
// Set only one pipeline for the renderer
// Pass transform from transformer system to create render

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSort,
	enum CameraType cameraType,
	union Camera camera,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation);
void destroyRenderer(
	struct Renderer* renderer);

struct Render* createRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Matrix4F model,
	struct Render* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	struct Render* render);

void executeRenderer(
	struct Renderer* renderer);

// TODO: create mesh render

struct Render* createTextRender(
	struct Renderer* renderer,
	bool render,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Matrix4F model,
	struct Render* parent,
	struct Text* text);
