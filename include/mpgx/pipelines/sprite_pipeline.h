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
#include "mpgx/window.h"

#define SPRITE_PIPELINE_NAME "Sprite"

Pipeline createExtSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	const PipelineState* state);
Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader);

Mat4F getSpritePipelineMvp(
	Pipeline pipeline);
void setSpritePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getSpritePipelineColor(
	Pipeline pipeline);
void setSpritePipelineColor(
	Pipeline pipeline,
	Vec4F color);
