#pragma once
#include "mpgx/window.h"

struct Font;
struct Text;

struct Font* createFont(
	const void* filePath);
void destroyFont(
	struct Font* font);

struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	struct Pipeline* pipeline,
	const char* data,
	bool constant);
void destroyText(
	struct Text* text);

struct Window* getTextWindow(
	const struct Text* text);
bool getTextConstant(
	const struct Text* text);

struct Font* getTextFont(
	const struct Text* text);
void setTextFont(
	struct Text* text,
	struct Font* font);

size_t getTextFontSize(
	const struct Text* text);
void setTextFontSize(
	struct Text* text,
	size_t fontSize);

struct Pipeline* getTextPipeline(
	const struct Text* text);
void setTextPipeline(
	struct Text* text,
	struct Pipeline* pipeline);

const char* getTextData(
	const struct Text* text);
bool setTextData(
	struct Text* text,
	const char* data);

bool updateText(
	struct Text* text,
	bool reuse);
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

