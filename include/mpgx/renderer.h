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

void drawRenderer(
	Renderer* renderer,
	Camera camera);

Render* createRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	void* handle);
void destroyRender(Render* render);

Renderer* getRenderRenderer(const Render* render);
Transform* getRenderTransform(const Render* render);

Box3F getRenderBounding(
	const Render* render);
void setRenderBounding(
	Render* render,
	Box3F bounding);

void* getRenderHandle(const Render* render);
