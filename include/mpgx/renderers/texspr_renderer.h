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
#include "mpgx/renderer.h"
#include "mpgx/pipelines/texspr_pipeline.h"

Renderer createTexSprRenderer(
	Transform transform,
	Pipeline pipeline,
	RenderSorting sortingType,
	bool useCulling,
	size_t capacity);
Render createTexSprRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh);

LinearColor getTexSprRenderColor(
	Render render);
void setTexSprRenderColor(
	Render render,
	LinearColor color);

Vec2F getTexSprRenderSize(
	Render render);
void setTexSprRenderSize(
	Render render,
	Vec2F size);

Vec2F getTexSprRenderOffset(
	Render render);
void setTexSprRenderOffset(
	Render render,
	Vec2F offset);

Mesh getTexSprRenderMesh(
	Render render);
void setTexSprRenderMesh(
	Render render,
	Mesh mesh);
