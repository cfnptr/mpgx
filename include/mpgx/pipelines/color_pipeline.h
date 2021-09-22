#pragma once
#include "mpgx/window.h"

#define COLOR_PIPELINE_NAME "Color"

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
