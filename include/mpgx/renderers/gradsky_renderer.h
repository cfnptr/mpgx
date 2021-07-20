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
	float sunHeight,
	Mesh mesh);

Vec4F getGradSkyRenderColor(
	Render render);
void setGradSkyRenderColor(
	Render render,
	Vec4F color);

float getGradSkyRenderSunHeight(
	Render render);
void setGradSkyRenderSunHeight(
	Render render,
	float sunHeight);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
