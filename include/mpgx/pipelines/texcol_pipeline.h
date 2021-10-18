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

#define TEXCOL_PIPELINE_NAME "TexCol"

Pipeline createExtTexColPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTexColPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getTexColPipelineTexture(
	Pipeline pipeline);
Sampler getTexColPipelineSampler(
	Pipeline pipeline);

Mat4F getTexColPipelineMvp(
	Pipeline pipeline);
void setTexColPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec2F getTexColPipelineSize(
	Pipeline pipeline);
void setTexColPipelineSize(
	Pipeline pipeline,
	Vec2F size);

Vec2F getTexColPipelineOffset(
	Pipeline pipeline);
void setTexColPipelineOffset(
	Pipeline pipeline,
	Vec2F offset);

Vec4F getTexColPipelineColor(
	Pipeline pipeline);
void setTexColPipelineColor(
	Pipeline pipeline,
	Vec4F color);
