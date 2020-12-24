#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

struct Renderer;
struct Render;

typedef void(*DestroyRender)(
	struct Render*);
typedef void(*RenderCommand)(
	struct Render*,
	struct Pipeline*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*);

struct Render
{
	struct Renderer* renderer;
	bool render;
	struct Transform* transform;
	DestroyRender destroyFunction;
	RenderCommand renderFunction;
	void* handle;
};

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSort,
	enum CameraType cameraType,
	union Camera camera,
	struct Transform* transform,
	struct Pipeline* pipeline);
void destroyRenderer(
	struct Renderer* renderer);

struct Render* createRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	struct Render* render);

void executeRenderer(
	struct Renderer* renderer);

struct Render* createMeshRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	struct Mesh* mesh);
struct Render* createTextRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	struct Text* text);
