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
	const char* _text,
	bool mipmap,
	bool constant);
void destroyText(
	struct Text* text);

struct Window* getTextWindow(
	const struct Text* text);
bool getTextMipmap(
	const struct Text* text);
bool getTextConstant(
	const struct Text* text);

struct Font* getTextFont(
	const struct Text* text);
bool setTextFont(
	struct Text* text,
	struct Font* font);

size_t getTextFontSize(
	const struct Text* text);
bool setTextFontSize(
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

// TODO:
// antialising on/off selector
// outline on/off selector

bool recreateText(
	struct Text* text);
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

