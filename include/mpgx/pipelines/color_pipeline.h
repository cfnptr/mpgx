#pragma once
#include "mpgx/window.h"

#define COLOR_PIPELINE_NAME "Color"

Pipeline createExtColorPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode,
	uint8_t polygonMode,
	uint8_t cullMode,
	uint8_t depthCompare,
	uint8_t colorWriteMask,
	bool cullFace,
	bool clockwiseFrontFace,
	bool testDepth,
	bool writeDepth,
	bool clampDepth,
	bool restartPrimitive,
	bool discardRasterizer,
	float lineWidth,
	Vec4I viewport,
	Vec2F depthRange,
	Vec4I scissor);
Pipeline createColorPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader);

Mat4F getColorPipelineMvp(
	Pipeline pipeline);
void setColorPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getColorPipelineColor(
	Pipeline pipeline);
void setColorPipelineColor(
	Pipeline pipeline,
	Vec4F color);
