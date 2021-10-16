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
#include "mpgx/interface.h"
#include <stdbool.h>

#define TEXT_PIPELINE_NAME "Text"

typedef struct Font* Font;
typedef struct Text* Text;

// TODO: add tab symbol handling

// TODO: add monochrome text support
// FT_LOAD_MONOCHROME

// TODO: add text coloring
// Pass color data to the vertex buffer

// TODO: add text fallback fonts

// TODO: add text mode ->
// generate texture once on init from string
// and use for rendering
// struct TextImage

Font createFont(
	const void* data,
	size_t size);
Font createFontFromFile(
	const void* filePath);
void destroyFont(Font font);

Text createText(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	const char* data,
	bool isConstant);
void destroyText(Text text);

Pipeline getTextPipeline(Text text);
bool isTextConstant(Text text);
Vec2F getTextSize(Text text);
size_t getTextIndexCount(Text text);

Vec2F getTextOffset(
	Text text,
	InterfaceAnchor anchor);
size_t getTextUnicodeCharCount(
	Text text);
bool getTextUnicodeCharAdvance(
	Text text,
	size_t index,
	Vec2F* advance);

Font getTextFont(
	Text text);
void setTextFont(
	Text text,
	Font font);

uint32_t getTextFontSize(
	Text text);
void setTextFontSize(
	Text text,
	uint32_t fontSize);

const char* getTextData(
	Text text);
bool setTextData(
	Text text,
	const char* data);

bool bakeText(
	Text text,
	bool reuse);
size_t drawText(Text text);

Sampler createTextSampler(Window window);

Pipeline createExtTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler);

Sampler getTextPipelineSampler(
	Pipeline pipeline);

Mat4F getTextPipelineMVP(
	Pipeline pipeline);
void setTextPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getTextPipelineColor(
	Pipeline pipeline);
void setTextPipelineColor(
	Pipeline pipeline,
	Vec4F color);
