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

Image getGradSkyPipelineTexture(
	Pipeline pipeline);
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline);

Mat4F getGradSkyPipelineMvp(
	Pipeline pipeline);
void setGradSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp);

Vec3F getGradSkyPipelineSunDir(
	Pipeline pipeline);
void setGradSkyPipelineSunDir(
	Pipeline pipeline,
	Vec3F sunDir);

Vec4F getGradSkyPipelineSunColor(
	Pipeline pipeline);
void setGradSkyPipelineSunColor(
	Pipeline pipeline,
	Vec4F sunColor);
