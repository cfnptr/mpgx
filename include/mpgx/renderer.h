// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
typedef struct RenderResult
{
	size_t renderCount;
	size_t indexCount;
	size_t passCount;
} RenderResult;

typedef void(*OnRenderHandleDestroy)(
	void* handle);
typedef size_t(*OnRenderHandleDraw)(
	Render render,
	Pipeline pipeline,
	const Mat4F* model,
	const Mat4F* viewProj);

Renderer createRenderer(
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	OnRenderHandleDestroy onHandleDestroy,
	OnRenderHandleDraw onHandleDraw,
	size_t capacity);
void destroyRenderer(Renderer renderer);

bool isRendererEmpty(Renderer renderer);
Pipeline getRendererPipeline(Renderer renderer);
OnRenderHandleDestroy getRendererOnHandleDestroy(Renderer renderer);
OnRenderHandleDraw getRendererOnHandleDraw(Renderer renderer);

RenderSorting getRendererSorting(
	Renderer renderer);
void setRendererSorting(
	Renderer renderer,
	RenderSorting sorting);

bool getRendererUseCulling(
	Renderer renderer);
void setRendererUseCulling(
	Renderer renderer,
	bool useCulling);

void enumerateRenderer(
	Renderer renderer,
	void(*onItem)(Render));

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
void destroyRender(
	Render render,
	bool destroyTransform);

Renderer getRenderRenderer(Render render);
Transform getRenderTransform(Render render);

Box3F getRenderBounding(
	Render render);
void setRenderBounding(
	Render render,
	Box3F bounding);

void* getRenderHandle(Render render);
