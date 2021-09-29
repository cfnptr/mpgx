#pragma once
#include "mpgx/window.h"

#define TEX_SPR_PIPELINE_NAME "TexSpr"

Pipeline createExtTexSprPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTexSprPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getTexSprPipelineTexture(
	Pipeline pipeline);
Sampler getTexSprPipelineSampler(
	Pipeline pipeline);

Mat4F getTexSprPipelineMvp(
	Pipeline pipeline);
void setTexSprPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec2F getTexSprPipelineSize(
	Pipeline pipeline);
void setTexSprPipelineSize(
	Pipeline pipeline,
	Vec2F size);

Vec2F getTexSprPipelineOffset(
	Pipeline pipeline);
void setTexSprPipelineOffset(
	Pipeline pipeline,
	Vec2F offset);

Vec4F getTexSprPipelineColor(
	Pipeline pipeline);
void setTexSprPipelineColor(
	Pipeline pipeline,
	Vec4F color);
