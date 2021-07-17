#pragma once
#include "mpgx/window.h"

Pipeline createColorPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode);

Shader getColorPipelineVertexShader(
	Pipeline pipeline);
Shader getColorPipelineFragmentShader(
	Pipeline pipeline);

Mat4F getColorPipelineMVP(
	Pipeline pipeline);
void setColorPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getColorPipelineColor(
	Pipeline pipeline);
void setColorPipelineColor(
	Pipeline pipeline,
	Vec4F color);
