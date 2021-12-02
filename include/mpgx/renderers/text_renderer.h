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
#include "mpgx/text.h"

Renderer createTextRenderer(
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity);
Render createTextRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Text text,
	Vec4U scissor);

LinearColor getTextRenderColor(
	Render render);
void setTextRenderColor(
	Render render,
	LinearColor color);

Text getTextRenderText(
	Render render);
void setTextRenderText(
	Render render,
	Text text);
