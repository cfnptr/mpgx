#pragma once
#include "mpgx/window.h"

// TODO: calc sky ambient color

Sampler createGradSkySampler(Window window);

Pipeline createGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	uint8_t drawMode);

Shader getGradSkyPipelineVertexShader(
	Pipeline pipeline);
Shader getGradSkyPipelineFragmentShader(
	Pipeline pipeline);
Image getGradSkyPipelineTexture(
	Pipeline pipeline);
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline);

Mat4F getGradSkyPipelineMVP(
	Pipeline pipeline);
void setGradSkyPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Vec4F getGradSkyPipelineColor(
	Pipeline pipeline);
void setGradSkyPipelineColor(
	Pipeline pipeline,
	Vec4F color);

float getGradSkyPipelineTime(
	Pipeline pipeline);
void setGradSkyPipelineTime(
	Pipeline pipeline,
	float time);
