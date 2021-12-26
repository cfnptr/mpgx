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
#include "mpgx/pipelines/texture_sprite_pipeline.h"

Renderer createTextureSpriteRenderer(
	Pipeline pipeline,
	RenderSorting sortingType,
	bool useCulling,
	size_t capacity);
Render createTextureSpriteRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	Mesh mesh);

LinearColor getTextureSpriteRenderColor(
	Render render);
void setTextureSpriteRenderColor(
	Render render,
	LinearColor color);

Vec2F getTextureSpriteRenderSize(
	Render render);
void setTextureSpriteRenderSize(
	Render render,
	Vec2F size);

Vec2F getTextureSpriteRenderOffset(
	Render render);
void setTextureSpriteRenderOffset(
	Render render,
	Vec2F offset);

Mesh getTextureSpriteRenderMesh(
	Render render);
void setTextureSpriteRenderMesh(
	Render render,
	Mesh mesh);
