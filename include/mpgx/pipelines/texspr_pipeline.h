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

#define TEX_SPR_PIPELINE_NAME "TexSpr"

Pipeline createExtTexSprPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTexSprPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getTexSprPipelineTexture(
	Pipeline pipeline);
Sampler getTexSprPipelineSampler(
	Pipeline pipeline);

Mat4F getTexSprPipelineMvp(
	Pipeline pipeline);
void setTexSprPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec2F getTexSprPipelineSize(
	Pipeline pipeline);
void setTexSprPipelineSize(
	Pipeline pipeline,
	Vec2F size);

Vec2F getTexSprPipelineOffset(
	Pipeline pipeline);
void setTexSprPipelineOffset(
	Pipeline pipeline,
	Vec2F offset);

Vec4F getTexSprPipelineColor(
	Pipeline pipeline);
void setTexSprPipelineColor(
	Pipeline pipeline,
	Vec4F color);
