#pragma once
#include "mpgx/window.h"
#include "mpgx/defines.h"

Pipeline* createColorPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getColorPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getColorPipelineFragmentShader(
	const Pipeline* pipeline);

Mat4F getColorPipelineMVP(
	const Pipeline* pipeline);
void setColorPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Vec4F getColorPipelineColor(
	const Pipeline* pipeline);
void setColorPipelineColor(
	Pipeline* pipeline,
	Vec4F color);

Pipeline* createSpritePipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getSpritePipelineVertexShader(
	const Pipeline* pipeline);
Shader* getSpritePipelineFragmentShader(
	const Pipeline* pipeline);

Mat4F getSpritePipelineMVP(
	const Pipeline* pipeline);
void setSpritePipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Vec4F getSpritePipelineColor(
	const Pipeline* pipeline);
void setSpritePipelineColor(
	Pipeline* pipeline,
	Vec4F color);

Pipeline* createDiffusePipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getDiffusePipelineVertexShader(
	const Pipeline* pipeline);
Shader* getDiffusePipelineFragmentShader(
	const Pipeline* pipeline);

Mat4F getDiffusePipelineMVP(
	const Pipeline* pipeline);
void setDiffusePipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Mat4F getDiffusePipelineNormal(
	const Pipeline* pipeline);
void setDiffusePipelineNormal(
	Pipeline* pipeline,
	Mat4F normal);

Vec4F getDiffusePipelineObjectColor(
	const Pipeline* pipeline);
void setDiffusePipelineObjectColor(
	Pipeline* pipeline,
	Vec4F objectColor);

Vec4F getDiffusePipelineAmbientColor(
	const Pipeline* pipeline);
void setDiffusePipelineAmbientColor(
	Pipeline* pipeline,
	Vec4F ambientColor);

Vec4F getDiffusePipelineLightColor(
	const Pipeline* pipeline);
void setDiffusePipelineLightColor(
	Pipeline* pipeline,
	Vec4F lightColor);

Vec3F getDiffusePipelineLightDirection(
	const Pipeline* pipeline);
void setDiffusePipelineLightDirection(
	Pipeline* pipeline,
	Vec3F lightDirection);
