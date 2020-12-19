#pragma once
#include "mpgx/window.h"

struct Pipeline* createColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize);

void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp);
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color);

struct Pipeline* createImageColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize,
	enum ImageFilter minFilter,
	enum ImageFilter magFilter,
	enum ImageFilter mipmapFilter,
	enum ImageWrap widthWrap,
	enum ImageWrap heightWrap,
	enum ImageWrap depthWrap,
	struct Image* image);

void setImageColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp);
void setImageColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color);
void setImageColorPipelineImage(
	struct Pipeline* pipeline,
	struct Image* image);

// TODO:
// 1. image filter/wrap gets
// 2. image offset/scale settings
