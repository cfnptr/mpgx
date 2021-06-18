#pragma once
#include "mpgx/window.h"

Pipeline* createColorPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getColorPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getColorPipelineFragmentShader(
	const Pipeline* pipeline);

Mat4F getColorPipelineMVP(
	const Pipeline* pipeline);
void setColorPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Vec4F getColorPipelineColor(
	const Pipeline* pipeline);
void setColorPipelineColor(
	Pipeline* pipeline,
	Vec4F color);
