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
#include "cmmt/color.h"

#define GRADIENT_SKY_PIPELINE_NAME "GradientSky"

typedef struct GradientSkyAmbient_T GradientSkyAmbient_T;
typedef GradientSkyAmbient_T* GradientSkyAmbient;

GradientSkyAmbient createGradientSkyAmbient(
	ImageData gradient);
void destroyGradientSkyAmbient(
	GradientSkyAmbient gradientSkyAmbient);

LinearColor getGradientSkyAmbientColor(
	GradientSkyAmbient gradientSkyAmbient,
	float dayTime);

Sampler createGradientSkySampler(Window window);

Pipeline createGradientSkyPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createGradientSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getGradientSkyPipelineTexture(
	Pipeline pipeline);
Sampler getGradientSkyPipelineSampler(
	Pipeline pipeline);

Mat4F getGradientSkyPipelineMvp(
	Pipeline pipeline);
void setGradientSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec3F getGradientSkyPipelineSunDir(
	Pipeline pipeline);
void setGradientSkyPipelineSunDir(
	Pipeline pipeline,
	Vec3F sunDir);

LinearColor getGradientSkyPipelineSunColor(
	Pipeline pipeline);
void setGradientSkyPipelineSunColor(
	Pipeline pipeline,
	LinearColor sunColor);
