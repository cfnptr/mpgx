#pragma once
#include "mpgx/window.h"

Pipeline* createTexColPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler,
	uint8_t drawMode);

Shader* getTexColPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getTexColPipelineFragmentShader(
	const Pipeline* pipeline);
Image* getTexColPipelineTexture(
	const Pipeline* pipeline);
Sampler* getTexColPipelineSampler(
	const Pipeline* pipeline);

Mat4F getTexColPipelineMVP(
	const Pipeline* pipeline);
void setTexColPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp);

Vec4F getTexColPipelineColor(
	const Pipeline* pipeline);
void setTexColPipelineColor(
	Pipeline* pipeline,
	Vec4F color);

Vec2F getTexColPipelineSize(
	const Pipeline* pipeline);
void setTexColPipelineSize(
	Pipeline* pipeline,
	Vec2F size);

Vec2F getTexColPipelineOffset(
	const Pipeline* pipeline);
void setTexColPipelineOffset(
	Pipeline* pipeline,
	Vec2F offset);
