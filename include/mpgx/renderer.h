#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/quaternion.h"

struct Renderer;
struct Render;

typedef void(*DestroyRender)(
	struct Render*);
typedef void(*DrawRenderCommand)(
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
	DrawRenderCommand drawFunction;
};

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
	DrawRenderCommand drawFunction);
void destroyRender(
	struct Render* render);

void drawRenderer(
	struct Renderer* renderer);
