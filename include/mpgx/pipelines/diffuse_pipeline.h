#pragma once
#include "mpgx/window.h"

Pipeline createDiffusePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode);

Shader getDiffusePipelineVertexShader(
	Pipeline pipeline);
Shader getDiffusePipelineFragmentShader(
	Pipeline pipeline);

Mat4F getDiffusePipelineMVP(
	Pipeline pipeline);
void setDiffusePipelineMVP(
	Pipeline pipeline,
	Mat4F mvp);

Mat4F getDiffusePipelineNormal(
	Pipeline pipeline);
void setDiffusePipelineNormal(
	Pipeline pipeline,
	Mat4F normal);

Vec4F getDiffusePipelineObjectColor(
	Pipeline pipeline);
void setDiffusePipelineObjectColor(
	Pipeline pipeline,
	Vec4F objectColor);

Vec4F getDiffusePipelineAmbientColor(
	Pipeline pipeline);
void setDiffusePipelineAmbientColor(
	Pipeline pipeline,
	Vec4F ambientColor);

Vec4F getDiffusePipelineLightColor(
	Pipeline pipeline);
void setDiffusePipelineLightColor(
	Pipeline pipeline,
	Vec4F lightColor);

Vec3F getDiffusePipelineLightDirection(
	Pipeline pipeline);
void setDiffusePipelineLightDirection(
	Pipeline pipeline,
	Vec3F lightDirection);
