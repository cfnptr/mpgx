#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/gradsky_pipeline.h"

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType);
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Mesh mesh);

Vec4F getGradSkyRenderColor(
	Render render);
void setGradSkyRenderColor(
	Render render,
	Vec4F color);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
