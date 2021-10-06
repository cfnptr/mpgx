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
#include "mpgx/pipelines/texcol_pipeline.h"

Renderer createTexColRenderer(
	Transform transform,
	Pipeline pipeline,
	RenderSorting sortingType,
	bool useCulling,
	size_t capacity);
Render createTexColRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh);

Vec4F getTexColRenderColor(
	Render render);
void setTexColRenderColor(
	Render render,
	Vec4F color);

Vec2F getTexColRenderSize(
	Render render);
void setTexColRenderSize(
	Render render,
	Vec2F size);

Vec2F getTexColRenderOffset(
	Render render);
void setTexColRenderOffset(
	Render render,
	Vec2F offset);

Mesh getTexColRenderMesh(
	Render render);
void setTexColRenderMesh(
	Render render,
	Mesh mesh);
