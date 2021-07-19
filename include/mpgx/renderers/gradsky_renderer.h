#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/gradsky_pipeline.h"

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	size_t capacity);
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	float time,
	Mesh mesh);

Vec4F getGradSkyRenderColor(
	Render render);
void setGradSkyRenderColor(
	Render render,
	Vec4F color);

float getGradSkyRenderTime(
	Render render);
void setGradSkyRenderTime(
	Render render,
	float time);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
