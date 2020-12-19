#pragma once
#include "mpgx/window.h"

struct Font;
struct Text;

struct Font* createFont(
	const void* fileData,
	size_t fileSize);
void destroyFont(
	struct Font* font);

// TODO: make also text buffer, text get/set
// Allow set text only if text is not constant

struct Text* createText(
	struct Window* window,
	struct Pipeline* pipeline,
	struct Font* font,
	size_t fontSize,
	const char* _text,
	bool mipmap,
	bool constant);
void destroyText(
	struct Text* text);

// TODO:
// get window,
// get/set pipeline,
// get/set font
// get/set text

// TODO:
// antialising on/off selector
// outline on/off selector

size_t getTextFontSize(
	const struct Text* text);
bool setTextFontSize(
	struct Text* text,
	size_t size);

// TODO:
// get/set text capacity
// get/set text texture capacity

void drawTextCommand(
	struct Text* text);

struct Pipeline* createTextPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize);

