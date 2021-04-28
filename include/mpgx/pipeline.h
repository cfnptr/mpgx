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
