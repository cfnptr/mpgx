#pragma once
#include "mpgx/window.h"

Pipeline createTexSprPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	uint8_t drawMode);

Shader getTexSprPipelineVertexShader(
	Pipeline pipeline);
Shader getTexSprPipelineFragmentShader(
	Pipeline pipeline);
Image getTexSprPipelineTexture(
	Pipeline pipeline);
Sampler getTexSprPipelineSampler(
	Pipeline pipeline);

Mat4F getTexSprPipelineMVP(
	Pipeline pipeline);
void setTexSprPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getTexSprPipelineColor(
	Pipeline pipeline);
void setTexSprPipelineColor(
	Pipeline pipeline,
	Vec4F color);

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
