#pragma once
#include "mpgx/window.h"

Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode);

Shader getSpritePipelineVertexShader(
	Pipeline pipeline);
Shader getSpritePipelineFragmentShader(
	Pipeline pipeline);

Mat4F getSpritePipelineMVP(
	Pipeline pipeline);
void setSpritePipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getSpritePipelineColor(
	Pipeline pipeline);
void setSpritePipelineColor(
	Pipeline pipeline,
	Vec4F color);
