#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/transformer.h"

#include "cmmt/camera.h"

typedef struct Renderer* Renderer;
typedef struct Render* Render;

typedef enum RenderSorting
{
	ASCENDING_RENDER_SORTING = 0,
	DESCENDING_RENDER_SORTING = 1,
	RENDER_SORTING_COUNT = 2,
} RenderSorting;

typedef struct RenderData
{
	Mat4F proj;
	Mat4F viewProj;
	Plane3F leftPlane;
	Plane3F rightPlane;
	Plane3F bottomPlane;
	Plane3F topPlane;
	Plane3F backPlane;
	Plane3F frontPlane;
} RenderData;
typedef struct RenderResult
{
	size_t renderCount;
	size_t indexCount;
} RenderResult;

typedef void(*OnRenderHandleDestroy)(
	void* handle);
typedef size_t(*OnRenderHandleDraw)(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj);

Renderer createRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	OnRenderHandleDestroy onHandleDestroy,
	OnRenderHandleDraw onHandleDraw,
	size_t capacity);
void destroyRenderer(Renderer renderer);

bool isRendererEmpty(Renderer renderer);
Transform getRendererTransform(Renderer renderer);
Pipeline getRendererPipeline(Renderer renderer);
OnRenderHandleDestroy getRendererOnHandleDestroy(Renderer renderer);
OnRenderHandleDraw getRendererOnHandleDraw(Renderer renderer);

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
	RenderData* data,
	bool createPlanes);
RenderResult drawRenderer(
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
