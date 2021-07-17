#pragma once
#include "mpgx/window.h"

Pipeline createTexColPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	uint8_t drawMode);

Shader getTexColPipelineVertexShader(
	Pipeline pipeline);
Shader getTexColPipelineFragmentShader(
	Pipeline pipeline);
Image getTexColPipelineTexture(
	Pipeline pipeline);
Sampler getTexColPipelineSampler(
	Pipeline pipeline);

Mat4F getTexColPipelineMVP(
	Pipeline pipeline);
void setTexColPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getTexColPipelineColor(
	Pipeline pipeline);
void setTexColPipelineColor(
	Pipeline pipeline,
	Vec4F color);

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
