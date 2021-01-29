#pragma once
#include "mpgx/window.h"

struct Font;
struct Text;

// TODO: add font loading from the memory

// TODO: add full language font creating support

struct Font* createFontFromFile(
	const void* filePath);
void destroyFont(
	struct Font* font);

struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
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

const char* getTextData(
	const struct Text* text);
bool setTextData(
	struct Text* text,
	const char* data);

// TODO: add monochrome text support

// TODO: add text coloring
// Pass color data to the vertex buffer

bool bakeText(
	struct Text* text,
	bool reuse);
void drawTextCommand(
	struct Text* text,
	struct Pipeline* pipeline);

// TODO:
// get uniChar offset for text cursor

struct Pipeline* createTextPipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader,
	uint8_t drawMode);

struct Shader* getTextPipelineVertexShader(
	const struct Pipeline* pipeline);
struct Shader* getTextPipelineFragmentShader(
	const struct Pipeline* pipeline);

struct Vector4F getTextPipelineColor(
	const struct Pipeline* pipeline);
void setTextPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color);

struct Matrix4F getTextPipelineMVP(
	const struct Pipeline* pipeline);
void setTextPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp);
