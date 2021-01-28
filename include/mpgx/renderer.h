#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

struct Renderer;
struct Render;

typedef void(*DestroyRender)(
	void* render);
typedef void(*RenderCommand)(
	struct Render* render,
	struct Pipeline* pipeline,
	const struct Matrix4F* model,
	const struct Matrix4F* view,
	const struct Matrix4F* proj,
	const struct Matrix4F* mvp);

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSorting,
	struct Pipeline* pipeline,
	struct Transformer* transformer,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent);
void destroyRenderer(
	struct Renderer* renderer);

struct Window* getRendererWindow(
	const struct Renderer* renderer);
bool isRendererAscendingSort(
	const struct Renderer* renderer);
struct Transformer* getRendererTransformer(
	const struct Renderer* renderer);
struct Transform* getRendererTransform(
	const struct Renderer* renderer);
struct Pipeline* getRendererPipeline(
	const struct Renderer* renderer);

void executeRenderer(
	struct Renderer* renderer,
	union Camera camera);

struct Render* createRender(
	struct Renderer* renderer,
	bool draw,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	struct Render* render);

struct Renderer* getRenderRenderer(
	const struct Render* render);
struct Transform* getRenderTransform(
	const struct Render* render);
struct Renderer* getRenderHandle(
	const struct Render* render);

bool getRenderDraw(
	const struct Render* render);
void setRenderDraw(
	struct Render* render,
	bool value);
