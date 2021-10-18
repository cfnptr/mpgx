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

#define GRADSKY_PIPELINE_NAME "GradSky"

typedef struct GradSkyAmbient* GradSkyAmbient;

GradSkyAmbient createGradSkyAmbient(ImageData gradient);
void destroyGradSkyAmbient(GradSkyAmbient gradSkyAmbient);

Vec4F getGradSkyAmbientColor(
	GradSkyAmbient gradSkyAmbient,
	float dayTime);

Sampler createGradSkySampler(Window window);

Pipeline createExtGradSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createGradSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getGradSkyPipelineTexture(
	Pipeline pipeline);
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline);

Mat4F getGradSkyPipelineMvp(
	Pipeline pipeline);
void setGradSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec3F getGradSkyPipelineSunDir(
	Pipeline pipeline);
void setGradSkyPipelineSunDir(
	Pipeline pipeline,
	Vec3F sunDir);

Vec4F getGradSkyPipelineSunColor(
	Pipeline pipeline);
void setGradSkyPipelineSunColor(
	Pipeline pipeline,
	Vec4F sunColor);
