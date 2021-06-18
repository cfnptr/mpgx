#pragma once
#include "mpgx/window.h"

Pipeline* createGradSkyPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getGradSkyPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getGradSkyPipelineFragmentShader(
	const Pipeline* pipeline);

Mat4F getGradSkyPipelineMVP(
	const Pipeline* pipeline);
void setGradSkyPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);
