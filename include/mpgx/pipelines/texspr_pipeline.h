#pragma once
#include "mpgx/window.h"

Pipeline* createTexSprPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler,
	uint8_t drawMode);

Shader* getTexSprPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getTexSprPipelineFragmentShader(
	const Pipeline* pipeline);
Image* getTexSprPipelineTexture(
	const Pipeline* pipeline);
Sampler* getTexSprPipelineSampler(
	const Pipeline* pipeline);

Mat4F getTexSprPipelineMVP(
	const Pipeline* pipeline);
void setTexSprPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Vec4F getTexSprPipelineColor(
	const Pipeline* pipeline);
void setTexSprPipelineColor(
	Pipeline* pipeline,
	Vec4F color);

Vec2F getTexSprPipelineSize(
	const Pipeline* pipeline);
void setTexSprPipelineSize(
	Pipeline* pipeline,
	Vec2F size);

Vec2F getTexSprPipelineOffset(
	const Pipeline* pipeline);
void setTexSprPipelineOffset(
	Pipeline* pipeline,
	Vec2F offset);