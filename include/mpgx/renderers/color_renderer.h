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
#include "mpgx/pipelines/color_pipeline.h"

Renderer createColorRenderer(
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity);
Render createColorRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Mesh mesh);

LinearColor getColorRenderColor(
	Render render);
void setColorRenderColor(
	Render render,
	LinearColor color);

Mesh getColorRenderMesh(
	Render render);
void setColorRenderMesh(
	Render render,
	Mesh mesh);
