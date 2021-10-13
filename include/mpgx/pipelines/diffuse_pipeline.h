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

#define DIFFUSE_PIPELINE_NAME "Diffuse"

Pipeline createExtDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const PipelineState* state);
Pipeline createDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader);

Mat4F getDiffusePipelineMvp(
	Pipeline pipeline);
void setDiffusePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Mat4F getDiffusePipelineNormal(
	Pipeline pipeline);
void setDiffusePipelineNormal(
	Pipeline pipeline,
	Mat4F normal);

Vec4F getDiffusePipelineObjectColor(
	Pipeline pipeline);
void setDiffusePipelineObjectColor(
	Pipeline pipeline,
	Vec4F objectColor);

Vec4F getDiffusePipelineAmbientColor(
	Pipeline pipeline);
void setDiffusePipelineAmbientColor(
	Pipeline pipeline,
	Vec4F ambientColor);

Vec4F getDiffusePipelineLightColor(
	Pipeline pipeline);
void setDiffusePipelineLightColor(
	Pipeline pipeline,
	Vec4F lightColor);

Vec3F getDiffusePipelineLightDirection(
	Pipeline pipeline);
void setDiffusePipelineLightDirection(
	Pipeline pipeline,
	Vec3F lightDirection);
