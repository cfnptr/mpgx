#pragma once
#include "mpgx/interface.h"
#include <stdbool.h>

#define TEXT_PIPELINE_NAME "Text"

typedef struct Font* Font;
typedef struct Text* Text;

// TODO: add font loading from the memory
// TODO: add tab symbol handling

// TODO: add monochrome text support
// FT_LOAD_MONOCHROME

// TODO: add text coloring
// Pass color data to the vertex buffer

// TODO: add text fallback fonts

Font createFontFromFile(
	const void* filePath);
void destroyFont(Font font);

Text createText(
	Window window,
	Font font,
	uint32_t fontSize,
	const char* data,
	bool isConstant);
void destroyText(Text text);

Window getTextWindow(Text text);
bool isTextConstant(Text text);
Vec2F getTextSize(Text text);
size_t getTextIndexCount(Text text);

Vec2F getTextOffset(
	Text text,
	uint8_t anchor);
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
size_t drawText(
	Text text,
	Pipeline pipeline);

Sampler createTextSampler(Window window);

Pipeline createExtTextPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTextPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler);

Sampler getTextPipelineSampler(
	Pipeline pipeline);

Vec4F getTextPipelineColor(
	Pipeline pipeline);
void setTextPipelineColor(
	Pipeline pipeline,
	Vec4F color);

Mat4F getTextPipelineMVP(
	Pipeline pipeline);
void setTextPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);
