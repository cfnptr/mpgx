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

#define BLOOM_PIPELINE_NAME "Bloom"

Pipeline createExtBloomPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const PipelineState* state);
Pipeline createBloomPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler);

Image getBloomPipelineBuffer(
	Pipeline pipeline);
Sampler getBloomPipelineSampler(
	Pipeline pipeline);

LinearColor getBloomPipelineThreshold(
	Pipeline pipeline);
void setBloomPipelineThreshold(
	Pipeline pipeline,
	LinearColor threshold);