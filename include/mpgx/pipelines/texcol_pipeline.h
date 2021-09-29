#pragma once
#include "mpgx/window.h"

#define TEX_COL_PIPELINE_NAME "TexCol"

Pipeline createExtTexColPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createTexColPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Image getTexColPipelineTexture(
	Pipeline pipeline);
Sampler getTexColPipelineSampler(
	Pipeline pipeline);

Mat4F getTexColPipelineMvp(
	Pipeline pipeline);
void setTexColPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec2F getTexColPipelineSize(
	Pipeline pipeline);
void setTexColPipelineSize(
	Pipeline pipeline,
	Vec2F size);

Vec2F getTexColPipelineOffset(
	Pipeline pipeline);
void setTexColPipelineOffset(
	Pipeline pipeline,
	Vec2F offset);

Vec4F getTexColPipelineColor(
	Pipeline pipeline);
void setTexColPipelineColor(
	Pipeline pipeline,
	Vec4F color);
