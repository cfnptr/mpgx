#pragma once
#include "mpgx/window.h"
#include <stdbool.h>

typedef struct Font Font;
typedef struct Text Text;

// TODO: add font loading from the memory
// TODO: add tab symbol handling

// TODO: add monochrome text support
// FT_LOAD_MONOCHROME

// TODO: add text coloring
// Pass color data to the vertex buffer

Font* createFontFromFile(
	const void* filePath);
void destroyFont(Font* font);

Text* createText(
	Window* window,
	Font* font,
	uint32_t fontSize,
	const char* data,
	bool isConstant);
void destroyText(Text* text);

Window* getTextWindow(const Text* text);
bool isTextConstant(const Text* text);
Vec2F getTextSize(const Text* text);

size_t getTextUnicodeCharCount(
	const Text* text);
bool getTextUnicodeCharAdvance(
	const Text* text,
	size_t index,
	Vec2F* advance);

Font* getTextFont(
	const Text* text);
void setTextFont(
	Text* text,
	Font* font);

uint32_t getTextFontSize(
	const Text* text);
void setTextFontSize(
	Text* text,
	uint32_t fontSize);

const char* getTextData(
	const Text* text);
bool setTextData(
	Text* text,
	const char* data);

bool bakeText(
	Text* text,
	bool reuse);
void drawText(
	Text* text,
	Pipeline* pipeline);

Pipeline* createTextPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getTextPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getTextPipelineFragmentShader(
	const Pipeline* pipeline);

Vec4F getTextPipelineColor(
	const Pipeline* pipeline);
void setTextPipelineColor(
	Pipeline* pipeline,
	Vec4F color);

Mat4F getTextPipelineMVP(
	const Pipeline* pipeline);
void setTextPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);
