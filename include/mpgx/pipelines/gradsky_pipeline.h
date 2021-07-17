#pragma once
#include "mpgx/window.h"

Pipeline createGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode);

Shader getGradSkyPipelineVertexShader(
	Pipeline pipeline);
Shader getGradSkyPipelineFragmentShader(
	Pipeline pipeline);

Mat4F getGradSkyPipelineMVP(
	Pipeline pipeline);
void setGradSkyPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);
