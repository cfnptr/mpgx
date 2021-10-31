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

#define GAUSSBLUR_PIPELINE_NAME "GaussBlur"

Pipeline createExtGaussBlurPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const PipelineState* state);
Pipeline createGaussBlurPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler);

Image getGaussBlurPipelineBuffer(
	Pipeline pipeline);
Sampler getGaussBlurPipelineSampler(
	Pipeline pipeline);

int getGaussBlurPipelineRadius(
	Pipeline pipeline);
void setGaussBlurPipelineRadius(
	Pipeline pipeline,
	int radius);