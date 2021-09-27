#pragma once
#include "mpgx/window.h"

#define SPRITE_PIPELINE_NAME "Sprite"

Pipeline createExtSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	const PipelineState* state);
Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader);

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
