#pragma once
#include "mpgx/window.h"
#include <stdbool.h>

typedef struct Font Font;
typedef struct Text Text;

// TODO: add font loading from the memory
// TODO: add tab symbol handling

Font* createFontFromFile(
	const void* filePath);
void destroyFont(
	Font* font);

Text* createText(
	Window* window,
	Font* font,
	size_t fontSize,
	const char* data,
	bool constant);
void destroyText(
	Text* text);

Window* getTextWindow(
	const Text* text);
bool isTextConstant(
	const Text* text);
size_t getTextUnicodeCharCount(
	const Text* text);
bool getTextUnicodeCharAdvance(
	const Text* text,
	size_t index,
	Vector2F* advance);

Font* getTextFont(
	const Text* text);
void setTextFont(
	Text* text,
	Font* font);

size_t getTextFontSize(
	const Text* text);
void setTextFontSize(
	Text* text,
	size_t fontSize);

const char* getTextData(
	const Text* text);
bool setTextData(
	Text* text,
	const char* data);

// TODO: add monochrome text support
// FT_LOAD_MONOCHROME

// TODO: add text coloring
// Pass color data to the vertex buffer

bool bakeText(
	Text* text,
	bool reuse);
void drawTextCommand(
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

Vector4F getTextPipelineColor(
	const Pipeline* pipeline);
void setTextPipelineColor(
	Pipeline* pipeline,
	Vector4F color);

Matrix4F getTextPipelineMVP(
	const Pipeline* pipeline);
void setTextPipelineMVP(
	Pipeline* pipeline,
	Matrix4F mvp);
