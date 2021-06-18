#pragma once
#include "mpgx/window.h"

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
