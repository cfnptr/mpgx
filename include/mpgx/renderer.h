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
	const struct Mat4F* model,
	const struct Mat4F* view,
	const struct Mat4F* proj,
	const struct Mat4F* mvp);

// TODO: frustum culling

struct Renderer* createRenderer(
	struct Pipeline* pipeline,
	struct Transformer* transformer,
	bool ascendingSorting,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Transform* parent);
void destroyRenderer(
	struct Renderer* renderer);

struct Transformer* getRendererTransformer(
	const struct Renderer* renderer);
struct Transform* getRendererTransform(
	const struct Renderer* renderer);
struct Pipeline* getRendererPipeline(
	const struct Renderer* renderer);

bool getRendererSorting(
	const struct Renderer* renderer);
void setRendererSorting(
	struct Renderer* renderer,
	bool ascendingSorting);

void executeRenderer(
	struct Renderer* renderer,
	union Camera camera);

struct Render* createRender(
	struct Renderer* renderer,
	bool draw,
	struct Vec3F position,
	struct Vec3F scale,
	struct Quat rotation,
	struct Render* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	struct Render* render);

struct Renderer* getRenderRenderer(
	const struct Render* render);
struct Transform* getRenderTransform(
	const struct Render* render);
void* getRenderHandle(
	const struct Render* render);

bool getRenderDraw(
	const struct Render* render);
void setRenderDraw(
	struct Render* render,
	bool value);

struct Vec3F getRenderPosition(
	const struct Render* render);
void setRenderPosition(
	struct Render* render,
	struct Vec3F position);

struct Vec3F getRenderScale(
	const struct Render* render);
void setRenderScale(
	struct Render* render,
	struct Vec3F scale);

struct Quat getRenderRotation(
	const struct Render* render);
void setRenderRotation(
	struct Render* render,
	struct Quat rotation);

struct Render* getRenderParent(
	const struct Render* render);
void setRenderParent(
	struct Render* render,
	struct Render* parent);
