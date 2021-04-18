#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

typedef struct Renderer Renderer;
typedef struct Render Render;

typedef void(*OnRenderDestroy)(
	void* render);
typedef void(*OnRenderDraw)(
	Render* render,
	Pipeline* pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp);

// TODO: frustum culling
// http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/

Renderer* createRenderer(
	Transform* transform,
	Pipeline* pipeline,
	bool ascendingSorting,
	OnRenderDestroy onDestroy,
	OnRenderDraw onDraw);
void destroyRenderer(Renderer* renderer);

Transform* getRendererTransform(
	const Renderer* renderer);
Pipeline* getRendererPipeline(
	const Renderer* renderer);
OnRenderDestroy getRendererOnDestroy(
	const Renderer* renderer);
OnRenderDraw getRendererOnDraw(
	const Renderer* renderer);

bool getRendererSorting(
	const Renderer* renderer);
void setRendererSorting(
	Renderer* renderer,
	bool ascendingSorting);

void updateRenderer(
	Renderer* renderer,
	Camera camera);

Render* createRender(
	Renderer* renderer,
	Transform* transform,
	void* handle);
void destroyRender(Render* render);

Renderer* getRenderRenderer(const Render* render);
Transform* getRenderTransform(const Render* render);
void* getRenderHandle(const Render* render);
