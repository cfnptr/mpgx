#pragma once
#include "mpgx/window.h"

#define SPRITE_PIPELINE_NAME "Sprite"

Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode);

Shader getSpritePipelineVertexShader(
	Pipeline pipeline);
Shader getSpritePipelineFragmentShader(
	Pipeline pipeline);

Mat4F getSpritePipelineMvp(
	Pipeline pipeline);
void setSpritePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getSpritePipelineColor(
	Pipeline pipeline);
void setSpritePipelineColor(
	Pipeline pipeline,
	Vec4F color);
