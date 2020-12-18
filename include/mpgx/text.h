#pragma once
#include "mpgx/window.h"

struct Font;
struct Text;

struct Font* createFont(
	const void* fileData,
	size_t fileSize);
void destroyFont(
	struct Font* font);

struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	const char* _text);
void destroyText(
	struct Text* text);

size_t getTextFontSize(
	struct Text* text);
bool setTextFontSize(
	struct Text* text,
	size_t size);

// TODO:
// get/set text capacity
// get/set text texture capacity

