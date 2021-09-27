#pragma once
#include "mpgx/window.h"

#define GRAD_SKY_PIPELINE_NAME "GradSky"

typedef struct GradSkyAmbient* GradSkyAmbient;

GradSkyAmbient createGradSkyAmbient(ImageData gradient);
void destroyGradSkyAmbient(GradSkyAmbient gradSkyAmbient);

Vec4F getGradSkyAmbientColor(
	GradSkyAmbient gradSkyAmbient,
	float dayTime);

Sampler createGradSkySampler(Window window);

Pipeline createExtGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state);
Pipeline createGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler);

Shader getGradSkyPipelineVertexShader(
	Pipeline pipeline);
Shader getGradSkyPipelineFragmentShader(
	Pipeline pipeline);
Image getGradSkyPipelineTexture(
	Pipeline pipeline);
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline);

Mat4F getGradSkyPipelineMvp(
	Pipeline pipeline);
void setGradSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

float getGradSkyPipelineSunHeight(
	Pipeline pipeline);
void setGradSkyPipelineSunHeight(
	Pipeline pipeline,
	float sunHeight);
