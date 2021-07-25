#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/transformer.h"

#include "cmmt/camera.h"

typedef struct Renderer* Renderer;
typedef struct Render* Render;

typedef enum RenderSorting
{
	NO_RENDER_SORTING = 0,
	ASCENDING_RENDER_SORTING = 1,
	DESCENDING_RENDER_SORTING = 2,
	RENDER_SORTING_COUNT = 3,
} RenderSorting;

typedef struct RenderData
{
	Mat4F view;
	Mat4F proj;
	Mat4F viewProj;
	Plane3F leftPlane;
	Plane3F rightPlane;
	Plane3F bottomPlane;
	Plane3F topPlane;
	Plane3F backPlane;
	Plane3F frontPlane;
} RenderData;

typedef void(*OnRenderDestroy)(
	void* render);
typedef void(*OnRenderDraw)(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* view,
	const Mat4F* proj,
	const Mat4F* viewProj,
	const Mat4F* mvp);

Renderer createRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	OnRenderDestroy onDestroy,
	OnRenderDraw onDraw,
	size_t capacity);
void destroyRenderer(Renderer renderer);

bool isRendererEmpty(Renderer renderer);
Transform getRendererTransform(Renderer renderer);
Pipeline getRendererPipeline(Renderer renderer);
OnRenderDestroy getRendererOnDestroy(Renderer renderer);
OnRenderDraw getRendererOnDraw(Renderer renderer);

uint8_t getRendererSortingType(
	Renderer renderer);
void setRendererSortingType(
	Renderer renderer,
	uint8_t sortingType);

bool getRendererUseCulling(
	Renderer renderer);
void setRendererUseCulling(
	Renderer renderer,
	bool useCulling);

void createRenderData(
	Window window,
	Mat4F view,
	Camera camera,
	RenderData* data);
void drawRenderer(
	Renderer renderer,
	const RenderData* data);

Render createRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	void* handle);
void destroyRender(Render render);

Renderer getRenderRenderer(Render render);
Transform getRenderTransform(Render render);

Box3F getRenderBounding(
	Render render);
void setRenderBounding(
	Render render,
	Box3F bounding);

void* getRenderHandle(Render render);
