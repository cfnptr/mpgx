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

#include <stdbool.h>
#define TEXT_PIPELINE_NAME "Text"

typedef struct Font* Font;
typedef struct Text* Text;

typedef enum AlignmentType
{
	CENTER_ALIGNMENT_TYPE = 0,
	LEFT_ALIGNMENT_TYPE = 1,
	RIGHT_ALIGNMENT_TYPE = 2,
	BOTTOM_ALIGNMENT_TYPE = 3,
	TOP_ALIGNMENT_TYPE = 4,
	LEFT_BOTTOM_ALIGNMENT_TYPE = 5,
	LEFT_TOP_ALIGNMENT_TYPE = 6,
	RIGHT_BOTTOM_ALIGNMENT_TYPE = 7,
	RIGHT_TOP_ALIGNMENT_TYPE = 8,
	ALIGNMENT_TYPE_COUNT = 9,
} AlignmentType;

bool createCharArray8(
	const uint32_t* data,
	size_t dataLength,
	char** array,
	size_t* arrayLength);
void destroyCharArray8(char* array);

bool createCharArray32(
	const char* data,
	size_t dataLength,
	uint32_t** array,
	size_t* arrayLength);
void destroyCharArray32(uint32_t* array);

Font createFont(
	const void* data,
	size_t size);
Font createFontFromFile(
	const void* filePath);
void destroyFont(Font font);

Text createText32(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const uint32_t* data,
	size_t dataLength,
	bool isConstant);
Text createText8(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const char* data,
	size_t dataLength,
	bool isConstant);
void destroyText(Text text);

Pipeline getTextPipeline(Text text);
bool isTextConstant(Text text);

Vec2F getTextSize(Text text);
Vec2F getTextOffset(Text text);

bool getTextCaretAdvance(
	Text text,
	size_t index,
	Vec2F* advance);
bool getTextCaretPosition(
	Text text,
	Vec2F* advance,
	size_t* index);

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

AlignmentType getTextAlignment(
	Text text);
void setTextAlignment(
	Text text,
	AlignmentType alignment);

size_t getTextDataLength(Text text);
const uint32_t* getTextData(Text text);

bool setTextData32(
	Text text,
	const uint32_t* data,
	size_t dataLength,
	bool reuseBuffers);
bool setTextData8(
	Text text,
	const char* data,
	size_t dataLength,
	bool reuseBuffers);

bool bakeText(
	Text text,
	bool reuseBuffers);
size_t drawText(
	Text text,
	Vec4U scissor);

float getTextPlatformScale(Pipeline pipeline);

Sampler createTextSampler(Window window);

Pipeline createExtTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const PipelineState* state,
	size_t textCapacity);
Pipeline createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	size_t textCapacity,
	bool useScissor);

Sampler getTextPipelineSampler(Pipeline pipeline);

Mat4F getTextPipelineMVP(
	Pipeline pipeline);
void setTextPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

LinearColor getTextPipelineColor(
	Pipeline pipeline);
void setTextPipelineColor(
	Pipeline pipeline,
	LinearColor color);

// TODO: add monochrome text support
// FT_LOAD_MONOCHROME

// TODO: add text coloring
// Pass color data to the vertex buffer

// TODO: add text fallback fonts

// TODO: add text mode ->
// generate texture once on init from string
// and use for rendering
// struct TextImage
